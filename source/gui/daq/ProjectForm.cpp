#include <gui/daq/ProjectForm.h>
#include "ui_ProjectForm.h"

#include <gui/Profiles.h>
#include <gui/widgets/QFileExtensions.h>
#include <gui/daq/ConsumerTemplatesForm.h>

#include <core/consumer_factory.h>
#include <core/util/timer.h>

#include <QSettings>
#include <QMessageBox>
#include <QCloseEvent>

#include <core/util/logger.h>

using namespace DAQuiri;

ProjectForm::ProjectForm(ThreadRunner& thread,
                         ProjectPtr proj,
                         QString identity,
                         QWidget* parent)
    : QWidget(parent)
      , ui(new Ui::ProjectForm)
      , runner_thread_(thread)
      , interruptor_(false)
      , project_(proj)
      , project_identity_(identity)
{
  ui->setupUi(this);

  menuLoad.addAction(QIcon(":/icons/oxy/16/document_open.png"), "Open daquiri project", this, SLOT(projectOpen()));
  ui->toolOpen->setMenu(&menuLoad);

  menuSave.addAction(QIcon(":/icons/oxy/16/document_save.png"), "Save project", this, SLOT(projectSave()));
  menuSave.addAction(QIcon(":/icons/oxy/16/document_save_as.png"), "Save project as...", this, SLOT(projectSaveAs()));
  menuSave.addAction(QIcon(":/icons/oxy/16/document_save_all.png"),
                     "Save as json + csv",
                     this,
                     SLOT(projectSaveSplit()));
  ui->toolSave->setMenu(&menuSave);
  ui->timeDuration->set_us_enabled(false);

  profile_dir_ = Profiles::singleton().current_profile_dir();

  if (!project_)
    project_ = ProjectPtr(new Project());
  plot_thread_.monitor_source(project_);
  ui->projectView->set_project(project_);
//  ui->projectView->setDetDB(detectors_);
  connect(&runner_thread_, SIGNAL(runComplete()), this, SLOT(run_completed()));
  connect(&plot_thread_, SIGNAL(plot_ready()), this, SLOT(update_plots()));

  loadSettings();

  on_pushForceRefresh_clicked();
}

QString ProjectForm::profile_dir() const
{
  return profile_dir_;
}

void ProjectForm::closeEvent(QCloseEvent* event)
{
  if (my_run_ && runner_thread_.running())
  {
    int reply = QMessageBox::warning(this, "Ongoing data acquisition",
                                     "Terminate?",
                                     QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
    {
      close_me_ = true;
      on_pushStop_clicked();
      return;
    }
    else
    {
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
                                     QMessageBox::Yes | QMessageBox::Cancel);
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

  if (!profile_dir_.isEmpty())
  {
    auto fname = profile_dir_ + "/default_consumers.daq";
    try
    {
      project_->open(fname.toStdString());
    }
    catch (std::exception& e)
    {
      DBG("<ProjectForm> Could not load default prototypes from {}\n{}",
          fname.toStdString(), hdf5::error::print_nested(e, 0));
    }
  }

  settings.beginGroup("Daq");
  ui->timeDuration->set_total_seconds(settings.value("run_secs", 60).toULongLong());
  ui->toggleIndefiniteRun->setChecked(settings.value("run_indefinite", true).toBool());
  ui->doubleSpinMinPause->setValue(settings.value("min_pause", 0.2).toDouble());
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
      DBG("Could not save default prototypes to {}", fname.toStdString());
    }
  }
}

void ProjectForm::toggle_push(bool enable, ProducerStatus status, StreamManifest manifest)
{
  stream_manifest_ = manifest;

  bool can_start = (status & ProducerStatus::can_run) && !my_run_;
  bool can_restart = (status & ProducerStatus::running) && my_run_;
  ui->pushStart->setEnabled(enable &&
      !project_->empty() &&
      (profile_dir_ == Profiles::singleton().current_profile_dir()) &&
      (can_start || can_restart));
  if (status & ProducerStatus::running)
  {
    ui->pushStart->setToolTip("Restart acquisition");
    ui->pushStart->setIcon(QIcon(":/icons/oxy/32/edit-redo.png"));
  }
  else
  {
    ui->pushStart->setToolTip("Start acquisition");
    ui->pushStart->setIcon(QIcon(":/icons/oxy/32/1rightarrow.png"));
  }

  ui->timeDuration->setEnabled(enable &&
      (status & ProducerStatus::can_run) &&
      !ui->toggleIndefiniteRun->isChecked());
  ui->toggleIndefiniteRun->setEnabled(enable &&
      (status & ProducerStatus::can_run));

  ui->toolOpen->setEnabled(enable && !my_run_);
  ui->toolSave->setEnabled(enable && project_->has_data() && !my_run_);
  ui->pushDetails->setVisible(false);
//  ui->pushDetails->setEnabled(enable && project_->has_data() && !my_run_);
  ui->projectView->set_manifest(manifest);

  ui->pushEditSpectra->setEnabled(enable && !my_run_);

  if (close_me_)
      emit requestClose(this);
}

void ProjectForm::clearGraphs() //rename this
{
  project_->clear();
  newProject();
  ui->projectView->set_project(project_); //wrong!!!!
  project_->activate();
}

QString ProjectForm::get_label() const
{
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

  return name;
}

void ProjectForm::update_plots()
{
  QString name = get_label();

  if (name != this->windowTitle())
  {
    this->setWindowTitle(name);
    emit toggleIO(true);
  }

  if (ui->projectView->isVisible())
    ui->projectView->update_plots();
}

void ProjectForm::projectOpen()
{
  QString formats = "daquiri project file (*.daq)";

  QString fileName = QFileDialog::getOpenFileName(this, "Load project",
                                                  data_directory_, formats, nullptr,
                                                  QFileDialog::DontUseNativeDialog);
  if (!validateFile(this, fileName, false))
    return;

  data_directory_ = path_of_file(fileName);

  //save first?
  if (project_->has_data())
  {
    int reply = QMessageBox::warning(this, "Clear existing?",
                                     "Spectra already open. Clear existing before opening?",
                                     QMessageBox::Yes | QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
      project_->clear();
    else
      return;
  }

  //toggle_push(false, false);
  INFO("Reading spectra from file {}", fileName.toStdString());
  clearGraphs();

  try
  {
    project_->open(fileName.toStdString());
  }
  catch (std::exception& e)
  {
    auto message = "Could not load project:\n" + hdf5::error::print_nested(e, 0);
    ERR("{}", message);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(QS(message));
    msgBox.exec();
    return;
  }

  newProject();

  project_identity_ = fileName;
  project_->activate();

  emit toggleIO(true);
}

void ProjectForm::projectSave()
{
  if (!project_identity_.isEmpty())
  {
    int reply = QMessageBox::warning(this, "Save?",
                                     "Save changes to existing project: "
                                         + project_identity_,
                                     QMessageBox::Yes | QMessageBox::Cancel);
    if (reply != QMessageBox::Yes)
      return;

    try
    {
      project_->save(project_identity_.toStdString());
    }
    catch (std::exception& e)
    {
      auto message = "Could not save project:\n" + hdf5::error::print_nested(e, 0);
      ERR("{}", message);
      QMessageBox msgBox;
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setText(QS(message));
      msgBox.exec();
      return;
    }

    update_plots();
  }
  else
    projectSaveAs();
}

void ProjectForm::save()
{
  if (project_identity_.isEmpty() || !project_->changed())
    return;

  boost::filesystem::path path(project_identity_.toStdString());
  if (!boost::filesystem::exists(path) || !hdf5::file::is_hdf5_file(path))
  {
    path = boost::filesystem::path(data_directory_.toStdString())
        / (project_identity_.toStdString() + ".daq");
  }

  try
  {
    INFO("Saving project to path {}", path.string());
    project_->save(path.string());
  }
  catch (std::exception& e)
  {
    auto message = "Could not save project:\n" + hdf5::error::print_nested(e, 0);
    ERR("{}", message);
    return;
  }

  project_identity_ = QS(path.string());
  update_plots();
}

void ProjectForm::projectSaveAs()
{
  QString formats = "daquiri project file (*.daq)";

  QString fileName = CustomSaveFileDialog(this, "Save project",
                                          data_directory_, formats);
  if (!validateFile(this, fileName, true))
    return;

  try
  {
    project_->save(fileName.toStdString());
  }
  catch (std::exception& e)
  {
    auto message = "Could not save project:\n" + hdf5::error::print_nested(e, 0);
    ERR("{}", message);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(QS(message));
    msgBox.exec();
    return;
  }

  project_identity_ = fileName;
  update_plots();
  data_directory_ = path_of_file(fileName);
}

void ProjectForm::projectSaveSplit()
{
  QString formats = "";

  QString fileName = CustomSaveFileDialog(this, "Save split",
                                          data_directory_, formats);
  if (validateFile(this, fileName + "_metadata.json", true))
  {
    INFO("Writing project to {}", fileName.toStdString());
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
  ui->projectView->set_project(project_);
  update_plots();
}

void ProjectForm::on_pushStop_clicked()
{
  ui->pushStop->setEnabled(false);
  interruptor_.store(true);
}

void ProjectForm::run_completed()
{
  if (my_run_)
  {
    //INFO( "ProjectForm received signal for run completed";
    ui->pushStop->setEnabled(false);
    my_run_ = false;

    update_plots();

    if (restarting_)
    {
      restarting_ = false;
      on_pushStart_clicked();
    }
    else
        emit toggleIO(true);
  }
}

void ProjectForm::on_pushStart_clicked()
{
  if (my_run_)
  {
    restarting_ = true;
    on_pushStop_clicked();
    return;
  }

  QSettings settings;
  settings.beginGroup("DAQ_behavior");
  auto do_what = settings.value("on_restart", "ask").toString();

  if (project_->has_data() && (do_what == "ask"))
  {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText("Project has data. Append, restart, or abort?");
    QPushButton* appendButton = msgBox.addButton(tr("Append"), QMessageBox::ActionRole);
    QPushButton* restartButton = msgBox.addButton(tr("Restart"), QMessageBox::ActionRole);
    QPushButton* abortButton = msgBox.addButton(QMessageBox::Abort);

    msgBox.exec();

    if (msgBox.clickedButton() == restartButton)
      do_what = "restart";
    else if (msgBox.clickedButton() == appendButton)
      do_what = "append";
    else if (msgBox.clickedButton() == abortButton)
      return;
  }

  if (do_what == "restart")
    project_->reset();

  start_DAQ();
}

void ProjectForm::start_DAQ()
{
  if (project_->empty())
    return;

  emit toggleIO(false);
  ui->pushStop->setEnabled(true);

  my_run_ = true;
  ui->projectView->set_project(project_);
  uint64_t duration = ui->timeDuration->total_seconds();
  if (ui->toggleIndefiniteRun->isChecked())
    duration = 0;
  runner_thread_.do_run(project_, interruptor_, duration);
}

void ProjectForm::newProject()
{
  ui->projectView->set_project(project_);
}

void ProjectForm::on_pushForceRefresh_clicked()
{
  ui->projectView->set_project(project_);
  update_plots();
}

//void ProjectForm::on_pushDetails_clicked()
//{
////  for (auto &q : project_->get_consumers())
////    DBG( "\n" << q.first << "=" << q.second->debug();

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

hr_time_t ProjectForm::opened() const
{
  return opened_;
}

bool ProjectForm::running() const
{
  return !interruptor_.load();
}
