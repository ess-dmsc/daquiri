#include "consumer_factory.h"
#include "ProjectForm.h"
#include "ui_ProjectForm.h"
#include "ConsumerTemplatesForm.h"
#include "custom_logger.h"
#include "custom_timer.h"
//#include "form_daq_settings.h"
#include "qt_util.h"
#include <QSettings>
#include <boost/filesystem.hpp>
#include <QMessageBox>

using namespace DAQuiri;

FormMcaDaq::FormMcaDaq(ThreadRunner &thread, Container<Detector>& detectors,
                       std::vector<Detector>& current_dets,
                       ProjectPtr proj, QWidget *parent) :
  QWidget(parent),
  interruptor_(false),
  project_(proj),
  runner_thread_(thread),
  detectors_(detectors),
  current_dets_(current_dets),
  my_run_(false),
  ui(new Ui::FormMcaDaq)
{
  ui->setupUi(this);

  if (!project_)
    project_ = ProjectPtr(new Project());
  else
    DBG << "project already exists";

  //connect with runner
  connect(&runner_thread_, SIGNAL(runComplete()), this, SLOT(run_completed()));

  //1d
  ui->Plot1d->setSpectra(project_);
  connect(&plot_thread_, SIGNAL(plot_ready()), this, SLOT(update_plots()));
//  ui->Plot1d->setDetDB(detectors_);

  //2d
  ui->Plot2d->setSpectra(project_);
//  ui->Plot2d->setDetDB(detectors_);

  menuLoad.addAction(QIcon(":/icons/oxy/16/document_open.png"), "Open qpx project", this, SLOT(projectOpen()));
  ui->toolOpen->setMenu(&menuLoad);

  menuSave.addAction(QIcon(":/icons/oxy/16/document_save.png"), "Save project", this, SLOT(projectSave()));
  menuSave.addAction(QIcon(":/icons/oxy/16/document_save_as.png"), "Save project as...", this, SLOT(projectSaveAs()));
  ui->toolSave->setMenu(&menuSave);

  this->setWindowTitle(QString::fromStdString(project_->identity()));

  ui->timeDuration->set_us_enabled(false);

  plot_thread_.monitor_source(project_);

  loadSettings();
}

FormMcaDaq::~FormMcaDaq()
{
}

void FormMcaDaq::closeEvent(QCloseEvent *event)
{
  if (my_run_ && runner_thread_.running())
  {
    int reply = QMessageBox::warning(this, "Ongoing data acquisition",
                                     "Terminate?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes)
    {
      runner_thread_.terminate();
      runner_thread_.wait();
    } else {
      event->ignore();
      return;
    }
  }

  if (project_->changed())
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

void FormMcaDaq::loadSettings() {
  QSettings settings_;

  settings_.beginGroup("Program");
  profile_directory_ = settings_.value("profile_directory", QDir::homePath() + "/qpx/settings").toString();
  data_directory_ = settings_.value("save_directory", QDir::homePath() + "/qpx/data").toString();
  settings_.endGroup();

//  spectra_templates_.read_xml(profile_directory_.toStdString() + "/default_sinks.tem");

  settings_.beginGroup("McaDaq");
  ui->timeDuration->set_total_seconds(settings_.value("run_secs", 60).toULongLong());
  ui->toggleIndefiniteRun->setChecked(settings_.value("run_indefinite", false).toBool());
  ui->timeDuration->setEnabled(!ui->toggleIndefiniteRun->isChecked());
  ui->pushEnable2d->setChecked(settings_.value("2d_visible", true).toBool());

  settings_.beginGroup("McaPlot");
  ui->Plot1d->set_scale_type(settings_.value("scale_type", "Logarithmic").toString());
  ui->Plot1d->set_plot_style(settings_.value("plot_style", "Step center").toString());
  settings_.endGroup();

  settings_.beginGroup("MatrixPlot");
  ui->Plot2d->set_zoom(settings_.value("zoom", 50).toDouble());
  ui->Plot2d->set_gradient(settings_.value("gradient", "Hot").toString());
  ui->Plot2d->set_scale_type(settings_.value("scale_type", "Logarithmic").toString());
  ui->Plot2d->set_show_legend(settings_.value("show_legend", false).toBool());
  settings_.endGroup();

  settings_.endGroup();

  on_pushEnable2d_clicked();
}

void FormMcaDaq::saveSettings() {
  QSettings settings_;

  settings_.beginGroup("Program");
  settings_.setValue("save_directory", data_directory_);
  settings_.endGroup();

  settings_.beginGroup("McaDaq");
  settings_.setValue("run_secs", QVariant::fromValue(ui->timeDuration->total_seconds()));
  settings_.setValue("run_indefinite", ui->toggleIndefiniteRun->isChecked());
  settings_.setValue("2d_visible", ui->pushEnable2d->isChecked());

  settings_.beginGroup("McaPlot");
  settings_.setValue("scale_type", ui->Plot1d->scale_type());
  settings_.setValue("plot_style", ui->Plot1d->plot_style());
  settings_.endGroup();

  settings_.beginGroup("MatrixPlot");
  settings_.setValue("zoom", ui->Plot2d->zoom());
  settings_.setValue("gradient", ui->Plot2d->gradient());
  settings_.setValue("scale_type", ui->Plot2d->scale_type());
  settings_.setValue("show_legend", ui->Plot2d->show_legend());
  settings_.endGroup();

  settings_.endGroup();
}

void FormMcaDaq::toggle_push(bool enable, ProducerStatus status) {
  bool online = (status & ProducerStatus::can_run);
  bool nonempty = !project_->empty();

  ui->pushMcaStart->setEnabled(enable && online && !my_run_);

  ui->timeDuration->setEnabled(enable && online && !ui->toggleIndefiniteRun->isChecked());
  ui->toggleIndefiniteRun->setEnabled(enable && online);

  ui->toolOpen->setEnabled(enable && !my_run_);
  ui->toolSave->setEnabled(enable && nonempty && !my_run_);
  ui->pushDetails->setEnabled(enable && nonempty && !my_run_);
}

void FormMcaDaq::clearGraphs() //rename this
{
  project_->clear();
  newProject();
  ui->Plot1d->reset_content();

  ui->Plot2d->reset_content(); //is this necessary?

  project_->activate();
}

void FormMcaDaq::update_plots()
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

  if (ui->Plot2d->isVisible())
  {
    this->setCursor(Qt::WaitCursor);
    ui->Plot2d->update_plot();
  }

  if (ui->Plot1d->isVisible())
  {
    this->setCursor(Qt::WaitCursor);
    ui->Plot1d->update_plot();
  }

  //ui->statusBar->showMessage("Spectra acquisition in progress...");
//  DBG << "<FormMcaDaq> Gui-side plotting " << guiside.ms() << " ms";
  this->setCursor(Qt::ArrowCursor);
}


void FormMcaDaq::projectSave()
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

void FormMcaDaq::projectSaveAs()
{
  QString formats = "qpx project file (*.qpx)";

  QString fileName = CustomSaveFileDialog(this, "Save project",
                                          data_directory_, formats);
  if (validateFile(this, fileName, true)) {
    LINFO << "Writing project to " << fileName.toStdString();
    this->setCursor(Qt::WaitCursor);
    project_->save_as(fileName.toStdString());
    update_plots();
  }

  data_directory_ = path_of_file(fileName);
}

void FormMcaDaq::on_pushEditSpectra_clicked()
{
  DialogSpectraTemplates* newDialog = new DialogSpectraTemplates(spectra_templates_, current_dets_,
                                                                 profile_directory_, this);
  newDialog->exec();
}

void FormMcaDaq::on_pushMcaStart_clicked()
{
  if (!project_->empty()) {
    int reply = QMessageBox::warning(this, "Continue?",
                                     "Non-empty spectra in project. Acquire and append to existing data?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply != QMessageBox::Yes)
      return;
    else
      start_DAQ();
  } else {
    DialogSpectraTemplates* newDialog = new DialogSpectraTemplates(spectra_templates_, current_dets_,
                                                                   profile_directory_, this);
    connect(newDialog, SIGNAL(accepted()), this, SLOT(start_DAQ()));
    newDialog->exec();
  }
}

void FormMcaDaq::start_DAQ()
{
  if (project_->empty() && spectra_templates_.empty())
    return;

  emit statusText("Spectra acquisition in progress...");

  emit toggleIO(false);
  ui->pushMcaStop->setEnabled(true);

  if (project_->empty()) {
    clearGraphs();
    project_->set_prototypes(spectra_templates_);
    newProject();
  }
//  project_->activate();

  my_run_ = true;
  ui->Plot1d->reset_content();
  uint64_t duration = ui->timeDuration->total_seconds();
  if (ui->toggleIndefiniteRun->isChecked())
    duration = 0;
  runner_thread_.do_run(project_, interruptor_, duration);
}


void FormMcaDaq::projectOpen()
{
  QString formats = "qpx project file (*.qpx)";

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
  LINFO << "Reading spectra from file " << fileName.toStdString();
  this->setCursor(Qt::WaitCursor);
  clearGraphs();

  project_->open(fileName.toStdString());

  newProject();
  project_->activate();

  emit toggleIO(true);

  this->setCursor(Qt::ArrowCursor);
}

void FormMcaDaq::updateSpectraUI()
{
  ui->Plot2d->updateUI();
  ui->Plot1d->setSpectra(project_);
}

void FormMcaDaq::newProject()
{
  ui->Plot2d->setSpectra(project_);
  ui->Plot2d->update_plot(true);
  ui->Plot1d->setSpectra(project_);
}

void FormMcaDaq::on_pushMcaStop_clicked()
{
  ui->pushMcaStop->setEnabled(false);
  //LINFO << "MCA acquisition interrupted by user";
  interruptor_.store(true);
}

void FormMcaDaq::run_completed()
{
  if (my_run_) {
    //LINFO << "FormMcaDaq received signal for run completed";
    ui->pushMcaStop->setEnabled(false);
    my_run_ = false;

    update_plots();

    emit toggleIO(true);
  }
}

void FormMcaDaq::replot() {
  update_plots();
}

void FormMcaDaq::on_pushEnable2d_clicked()
{
  if (ui->pushEnable2d->isChecked()) {
    ui->Plot2d->show();
    update_plots();
  } else
    ui->Plot2d->hide();
  ui->line_sep2d->setVisible(ui->pushEnable2d->isChecked());
}

void FormMcaDaq::on_pushForceRefresh_clicked()
{
  updateSpectraUI();
  update_plots();
}

//void FormMcaDaq::on_pushDetails_clicked()
//{
////  for (auto &q : project_->get_sinks())
////    DBG << "\n" << q.first << "=" << q.second->debug();

//  FormDaqSettings *DaqInfo = new FormDaqSettings(project_, this);
//  DaqInfo->setWindowTitle("System settings at the time of acquisition");
//  DaqInfo->exec();
//}

void FormMcaDaq::on_toggleIndefiniteRun_clicked()
{
   ui->timeDuration->setEnabled(!ui->toggleIndefiniteRun->isChecked());
}
