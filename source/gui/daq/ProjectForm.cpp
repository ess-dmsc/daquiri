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
  ui->Plot1d->setSpectra(project_);
  connect(&plot_thread_, SIGNAL(plot_ready()), this, SLOT(update_plots()));
//  ui->Plot1d->setDetDB(detectors_);

  menuLoad.addAction(QIcon(":/icons/oxy/16/document_open.png"), "Open daquiri project", this, SLOT(projectOpen()));
  ui->toolOpen->setMenu(&menuLoad);

  menuSave.addAction(QIcon(":/icons/oxy/16/document_save.png"), "Save project", this, SLOT(projectSave()));
  menuSave.addAction(QIcon(":/icons/oxy/16/document_save_as.png"), "Save project as...", this, SLOT(projectSaveAs()));
  ui->toolSave->setMenu(&menuSave);

  this->setWindowTitle(QString::fromStdString(project_->identity()));

  ui->timeDuration->set_us_enabled(false);

  plot_thread_.monitor_source(project_);

  loadSettings();
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

  spectra_templates_ =
      from_json_file(Profiles::current_profile_dir().toStdString() + "/default_sinks.tem");

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
  if (settings.value("autosave_daq", true).toBool()
      && !project_->empty())
  {
    spectra_templates_.clear();
    for (auto &q : project_->get_sinks())
      spectra_templates_.add_a(q.second->metadata().prototype());
    to_json_file(spectra_templates_,
                 Profiles::current_profile_dir().toStdString() + "/default_sinks.tem");
  }
}

void ProjectForm::toggle_push(bool enable, ProducerStatus status)
{
  bool online = (status & ProducerStatus::can_run);
  bool nonempty = !project_->empty();

  ui->pushStart->setEnabled(enable && online && !my_run_);

  ui->timeDuration->setEnabled(enable && online && !ui->toggleIndefiniteRun->isChecked());
  ui->toggleIndefiniteRun->setEnabled(enable && online);

  ui->toolOpen->setEnabled(enable && !my_run_);
  ui->toolSave->setEnabled(enable && nonempty && !my_run_);
  ui->pushDetails->setEnabled(enable && nonempty && !my_run_);

  if (close_me_)
    emit requestClose(this);
}

void ProjectForm::clearGraphs() //rename this
{
  project_->clear();
  newProject();
  ui->Plot1d->setSpectra(project_); //wrong!!!!
  project_->activate();
}

void ProjectForm::update_plots()
{
  //ui->statusBar->showMessage("Updating plots");

  CustomTimer guiside(true);

  QString name = QString::fromStdString(project_->identity());
  if (name != "New project")
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

  ui->pushEditSpectra->setVisible(project_->empty());

  if (ui->Plot1d->isVisible())
  {
    this->setCursor(Qt::WaitCursor);
    ui->Plot1d->update_plots();
  }

  //ui->statusBar->showMessage("Spectra acquisition in progress...");
//  DBG << "<ProjectForm> Gui-side plotting " << guiside.ms() << " ms";
  this->setCursor(Qt::ArrowCursor);
}


void ProjectForm::projectSave()
{
  if (project_->changed() && (project_->identity() != "New project")) {
    int reply = QMessageBox::warning(this, "Save?",
                                     "Save changes to existing project: " + QString::fromStdString(project_->identity()),
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
      this->setCursor(Qt::WaitCursor);
      project_->save();
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
  if (validateFile(this, fileName, true)) {
    INFO << "Writing project to " << fileName.toStdString();
    this->setCursor(Qt::WaitCursor);
    project_->save_as(fileName.toStdString());
    update_plots();
  }

  data_directory_ = path_of_file(fileName);
}

void ProjectForm::on_pushEditSpectra_clicked()
{
  ConsumerTemplatesForm* newDialog =
      new ConsumerTemplatesForm(spectra_templates_,
                                current_dets_,
                                Profiles::current_profile_dir(),
                                this);
  newDialog->exec();
}

void ProjectForm::on_pushStart_clicked()
{
  if (!project_->empty())
  {
    int reply = QMessageBox::warning(this, "Continue?",
                                     "Non-empty spectra in project. Acquire and append to existing data?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply != QMessageBox::Yes)
      return;
    else
      start_DAQ();
  }
  else
  {
    QSettings settings;
    settings.beginGroup("DAQ_behavior");
    if (settings.value("confirm_templates", true).toBool())
    {
      ConsumerTemplatesForm* newDialog =
          new ConsumerTemplatesForm(spectra_templates_,
                                    current_dets_,
                                    Profiles::current_profile_dir(),
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
  if (project_->empty() && spectra_templates_.empty())
    return;

  emit toggleIO(false);
  ui->pushStop->setEnabled(true);

  if (project_->empty())
  {
    clearGraphs();
    project_->set_prototypes(spectra_templates_);
    newProject();
  }
//  project_->activate();

  my_run_ = true;
  ui->Plot1d->setSpectra(project_);
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
  if (!project_->empty()) {
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
  this->setCursor(Qt::WaitCursor);
  clearGraphs();

  project_->open(fileName.toStdString());

  newProject();
  project_->activate();

  emit toggleIO(true);

  this->setCursor(Qt::ArrowCursor);
}

void ProjectForm::newProject()
{
  ui->Plot1d->setSpectra(project_);
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
  ui->Plot1d->setSpectra(project_);
  update_plots();
}

//void ProjectForm::on_pushDetails_clicked()
//{
////  for (auto &q : project_->get_sinks())
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
