#include "consumer_factory.h"
#include "ProjectForm.h"
#include "ui_ProjectForm.h"
#include "ConsumerTemplatesForm.h"
#include "custom_logger.h"
#include "custom_timer.h"
//#include "form_daq_settings.h"
//#include "qt_util.h"
#include <QSettings>
#include <boost/filesystem.hpp>
#include <QMessageBox>
#include "json_file.h"

#include <QCloseEvent>

#include "Profiles.h"
#include "QFileExtensions.h"

using namespace DAQuiri;

ProjectForm::ProjectForm(ThreadRunner &thread, Container<Detector>& detectors,
                       std::vector<Detector>& current_dets,
                       ProjectPtr proj, QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::ProjectForm)
  , runner_thread_(thread)
  , interruptor_(false)
  , project_(proj)
  , detectors_(detectors)
  , current_dets_(current_dets)
{
  ui->setupUi(this);

  if (!project_)
    project_ = ProjectPtr(new Project());
//  else
//    DBG << "project already exists";

  //connect with runner
  connect(&runner_thread_, SIGNAL(runComplete()), this, SLOT(run_completed()));

  //1d
  ui->projectView->setSpectra(project_);
  connect(&plot_thread_, SIGNAL(plot_ready()), this, SLOT(update_plots()));
//  ui->projectView->setDetDB(detectors_);

  menuLoad.addAction(QIcon(":/icons/oxy/16/document_open.png"), "Open daquiri project", this, SLOT(projectOpen()));
  ui->toolOpen->setMenu(&menuLoad);

  menuSave.addAction(QIcon(":/icons/oxy/16/document_save.png"), "Save project", this, SLOT(projectSave()));
  menuSave.addAction(QIcon(":/icons/oxy/16/document_save_as.png"), "Save project as...", this, SLOT(projectSaveAs()));
  menuSave.addAction(QIcon(":/icons/oxy/16/document_save_all.png"), "Save as json + csv", this, SLOT(projectSaveSplit()));
  ui->toolSave->setMenu(&menuSave);

  this->setWindowTitle("New project");

  ui->timeDuration->set_us_enabled(false);

  plot_thread_.monitor_source(project_);

  loadSettings();
  update_plots();
}

ProjectForm::~ProjectForm()
{
//  delete ui;
}

void ProjectForm::closeEvent(QCloseEvent *event)
{
  if (my_run_ && runner_thread_.running())
  {
    int reply = QMessageBox::warning(this, "Ongoing data acquisition",
                                     "Terminate?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
    {
      close_me_ = true;
      on_pushStop_clicked();
//      interruptor_.store(true);
//      runner_thread_.terminate();
//      runner_thread_.wait();
//      emit toggleIO(true);
      return;
    } else {
      event->ignore();
      return;
    }
  }

  close_me_ = false;
  QSettings settings;
  settings.beginGroup("DAQ_behavior");

  if (project_->changed() && settings.value("ask_save_project", true).toBool())
  {
    int reply = QMessageBox::warning(this, "Project contents changed",
                                     "Discard?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply != QMessageBox::Yes)
    {
      event->ignore();
      return;
    }
  }

  plot_thread_.terminate_wait();
  saveSettings();
  event->accept();
}

void ProjectForm::loadSettings()
{
  QSettings settings;

  settings.beginGroup("Program");
  data_directory_ = settings.value("save_directory", QDir::currentPath()).toString();
  settings.endGroup();

  profile_dir_ = Profiles::current_profile_dir();

  if (!profile_dir_.isEmpty())
  {
    auto fname = profile_dir_ + "/default_consumers.daq";
    try
    {
      project_->open(fname.toStdString());
    }
    catch (...)
    {
      DBG << "Could not load default prototypes from " << fname.toStdString();
    }
  }

  settings.beginGroup("Daq");
  ui->timeDuration->set_total_seconds(settings.value("run_secs", 60).toULongLong());
  ui->toggleIndefiniteRun->setChecked(settings.value("run_indefinite", false).toBool());
  ui->doubleSpinMinPause->setValue(settings.value("min_pause", 1.0).toDouble());
  ui->timeDuration->setEnabled(!ui->toggleIndefiniteRun->isChecked());

  on_doubleSpinMinPause_editingFinished();
  settings.endGroup();
}

void ProjectForm::saveSettings()
{
  QSettings settings;

  settings.beginGroup("Program");
  settings.setValue("save_directory", data_directory_);
  settings.endGroup();

  settings.beginGroup("Daq");
  settings.setValue("run_secs", QVariant::fromValue(ui->timeDuration->total_seconds()));
  settings.setValue("run_indefinite", ui->toggleIndefiniteRun->isChecked());
  settings.setValue("min_pause", ui->doubleSpinMinPause->value());
  settings.endGroup();

  settings.beginGroup("DAQ_behavior");
  if (settings.value("autosave_daq", true).toBool() && !profile_dir_.isEmpty())
  {
    project_->reset();
    auto fname = profile_dir_ + "/default_consumers.daq";
    try
    {
      project_->save(fname.toStdString());
    }
    catch (...)
    {
      DBG << "Could not save default prototypes to " << fname.toStdString();
    }
  }
}

void ProjectForm::toggle_push(bool enable, ProducerStatus status, StreamManifest manifest)
{
  stream_manifest_ = manifest;
  bool online = (status & ProducerStatus::can_run);
  bool has_data = project_->has_data();

  ui->pushStart->setEnabled(enable && online && !my_run_);

  ui->timeDuration->setEnabled(enable && online && !ui->toggleIndefiniteRun->isChecked());
  ui->toggleIndefiniteRun->setEnabled(enable && online);

  ui->toolOpen->setEnabled(enable && !my_run_);
  ui->toolSave->setEnabled(enable && has_data && !my_run_);
  ui->pushDetails->setEnabled(enable && has_data && !my_run_);
  ui->projectView->set_manifest(manifest);

  ui->pushEditSpectra->setEnabled(enable && !my_run_);

  if (close_me_)
    emit requestClose(this);
}

void ProjectForm::clearGraphs() //rename this
{
  project_->clear();
  newProject();
  ui->projectView->setSpectra(project_); //wrong!!!!
  project_->activate();
}

void ProjectForm::update_plots()
{
  //ui->statusBar->showMessage("Updating plots");

  CustomTimer guiside(true);

  QString name = project_identity_;
  if (name.isEmpty())
  {
    name = "New project";
  }
  else
  {
    QStringList slist = name.split("/");
    if (!slist.empty())
      name = slist.back();
  }

  if (my_run_)
    name += QString::fromUtf8("  \u25b6");
  else if (project_->changed())
    name += QString::fromUtf8(" \u2731");

  if (name != this->windowTitle())
  {
    this->setWindowTitle(name);
    emit toggleIO(true);
  }

  if (ui->projectView->isVisible())
    ui->projectView->update_plots();

  //ui->statusBar->showMessage("Spectra acquisition in progress...");
//  DBG << "<ProjectForm> Gui-side plotting " << guiside.ms() << " ms";
}


void ProjectForm::projectSave()
{
  if (!project_identity_.isEmpty())
  {
    int reply = QMessageBox::warning(this, "Save?",
                                     "Save changes to existing project: "
                                     + project_identity_,
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
    {
      project_->save(project_identity_.toStdString());
      update_plots();
    }
  } else
    projectSaveAs();
}

void ProjectForm::projectSaveAs()
{
  QString formats = "daquiri project file (*.daq)";

  QString fileName = CustomSaveFileDialog(this, "Save project",
                                          data_directory_, formats);
  if (validateFile(this, fileName, true))
  {
    project_->save(fileName.toStdString());
    project_identity_ = fileName;
    update_plots();
  }

  data_directory_ = path_of_file(fileName);
}

void ProjectForm::projectSaveSplit()
{
  QString formats = "";

  QString fileName = CustomSaveFileDialog(this, "Save split",
                                          data_directory_, formats);
  if (validateFile(this, fileName+ "_metadata.json", true))
  {
    INFO << "Writing project to " << fileName.toStdString();
    project_->save_split(fileName.toStdString());
    update_plots();
  }

  data_directory_ = path_of_file(fileName);
}

void ProjectForm::on_pushEditSpectra_clicked()
{
  ConsumerTemplatesForm* newDialog =
      new ConsumerTemplatesForm(project_,
                                current_dets_,
                                stream_manifest_,
                                data_directory_,
                                profile_dir_,
                                this);
  newDialog->exec();
  ui->projectView->setSpectra(project_);
  update_plots();
}

void ProjectForm::on_pushStart_clicked()
{
  if (project_->has_data())
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("Project has data. Append, restart, or abort?");
    QPushButton *appendButton = msgBox.addButton(tr("Append"), QMessageBox::ActionRole);
    QPushButton *restartButton = msgBox.addButton(tr("Restart"), QMessageBox::ActionRole);
    QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);

    msgBox.exec();

    if (msgBox.clickedButton() == restartButton)
    {
      project_->reset();
      start_DAQ();
    }
    else if (msgBox.clickedButton() == appendButton)
    {
      start_DAQ();
    }
    else if (msgBox.clickedButton() == abortButton)
    {
      return;
    }

  }
  else
  {
    QSettings settings;
    settings.beginGroup("DAQ_behavior");
    if (settings.value("confirm_templates", true).toBool())
    {
      ConsumerTemplatesForm* newDialog =
          new ConsumerTemplatesForm(project_,
                                    current_dets_,
                                    stream_manifest_,
                                    data_directory_,
                                    profile_dir_,
                                    this);
      connect(newDialog, SIGNAL(accepted()), this, SLOT(start_DAQ()));
      newDialog->exec();
    }
    else
      start_DAQ();
  }
}

void ProjectForm::start_DAQ()
{
  if (project_->empty())
    return;

  emit toggleIO(false);
  ui->pushStop->setEnabled(true);

  my_run_ = true;
  ui->projectView->setSpectra(project_);
  uint64_t duration = ui->timeDuration->total_seconds();
  if (ui->toggleIndefiniteRun->isChecked())
    duration = 0;
  runner_thread_.do_run(project_, interruptor_, duration);
}


void ProjectForm::projectOpen()
{
  QString formats = "daquiri project file (*.daq)";

  QString fileName = QFileDialog::getOpenFileName(this, "Load project",
                                                  data_directory_, formats);
  if (!validateFile(this, fileName, false))
    return;

  data_directory_ = path_of_file(fileName);

  //save first?
  if (project_->has_data()) {
    int reply = QMessageBox::warning(this, "Clear existing?",
                                     "Spectra already open. Clear existing before opening?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
      project_->clear();
    else
      return;
  }

  //toggle_push(false, false);
  INFO << "Reading spectra from file " << fileName.toStdString();
  clearGraphs();

  project_->open(fileName.toStdString());

  newProject();

  project_identity_ = fileName;
  project_->activate();

  emit toggleIO(true);
}

void ProjectForm::newProject()
{
  ui->projectView->setSpectra(project_);
}

void ProjectForm::on_pushStop_clicked()
{
  ui->pushStop->setEnabled(false);
  //INFO << " acquisition interrupted by user";
  interruptor_.store(true);
}

void ProjectForm::run_completed()
{
  if (my_run_) {
    //INFO << "ProjectForm received signal for run completed";
    ui->pushStop->setEnabled(false);
    my_run_ = false;

    update_plots();

    emit toggleIO(true);
  }
}

//void ProjectForm::replot()
//{
//  update_plots();
//}

void ProjectForm::on_pushForceRefresh_clicked()
{
  ui->projectView->setSpectra(project_);
  update_plots();
}

//void ProjectForm::on_pushDetails_clicked()
//{
////  for (auto &q : project_->get_consumers())
////    DBG << "\n" << q.first << "=" << q.second->debug();

//  FormDaqSettings *DaqInfo = new FormDaqSettings(project_, this);
//  DaqInfo->setWindowTitle("System settings at the time of acquisition");
//  DaqInfo->exec();
//}

void ProjectForm::on_toggleIndefiniteRun_clicked()
{
   ui->timeDuration->setEnabled(!ui->toggleIndefiniteRun->isChecked());
}

void ProjectForm::on_doubleSpinMinPause_editingFinished()
{
  plot_thread_.set_wait_time(ui->doubleSpinMinPause->value() * 1000);
}
