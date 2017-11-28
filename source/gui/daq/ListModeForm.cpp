#include "ListModeForm.h"
#include "ui_ListModeForm.h"
#include "custom_logger.h"
#include "qt_util.h"
#include <QSettings>
#include <QMessageBox>

#include "lexical_extensions.h"

using namespace DAQuiri;

ListModeForm::ListModeForm(ThreadRunner &thread, QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::ListModeForm)
  , runner_thread_(thread)
  , interruptor_(false)
  , my_run_(false)
  , attr_model_(this)
{
  ui->setupUi(this);

  this->setWindowTitle("List LIVE");

  loadSettings();

  connect(&runner_thread_, SIGNAL(listComplete(DAQuiri::ListData)), this,
          SLOT(list_completed(DAQuiri::ListData)));

  connect(ui->listSpills, SIGNAL(currentRowChanged(int)), this, SLOT(spillSelectionChanged(int)));

  ui->tableHits->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->tableHits->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableHits->horizontalHeader()->setStretchLastSection(true);
  ui->tableHits->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(ui->tableHits->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(event_selection_changed(QItemSelection,QItemSelection)));

  ui->tableHitValues->setSelectionMode(QAbstractItemView::NoSelection);
  ui->tableHitValues->horizontalHeader()->setStretchLastSection(true);
  ui->tableHitValues->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  ui->treeAttribs->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->treeAttribs->setModel(&attr_model_);
  ui->treeAttribs->setItemDelegate(&attr_delegate_);
  ui->treeAttribs->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  attr_model_.set_show_address_(false);

  ui->timeDuration->set_us_enabled(false);

  ui->treeAttribs->setVisible(false);
  ui->labelState->setVisible(false);

//  ui->labelEvents->setVisible(false);
//  ui->tableHits->setVisible(false);
//  ui->labelEventVals->setVisible(false);
//  ui->tableHitValues->setVisible(false);
}

void ListModeForm::loadSettings() {
  QSettings settings_;

  settings_.beginGroup("ListDaq");
  ui->timeDuration->set_total_seconds(settings_.value("run_secs", 60).toULongLong());
  settings_.endGroup();
}

void ListModeForm::saveSettings() {
  QSettings settings_;

  settings_.beginGroup("ListDaq");
  settings_.setValue("run_secs", QVariant::fromValue(ui->timeDuration->total_seconds()));
  settings_.endGroup();
}

void ListModeForm::toggle_push(bool enable, ProducerStatus status) {
  bool online = (status & ProducerStatus::can_run);
  ui->pushListStart->setEnabled(enable && online);
  ui->timeDuration->setEnabled(enable && online);
}

ListModeForm::~ListModeForm()
{
  delete ui;
}

void ListModeForm::closeEvent(QCloseEvent *event) {
  if (my_run_ && runner_thread_.running()) {
    int reply = QMessageBox::warning(this, "Ongoing data acquisition",
                                     "Terminate?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
      runner_thread_.terminate();
      runner_thread_.wait();
    } else {
      event->ignore();
      return;
    }
  }


  if (!list_data_.empty()) {
    int reply = QMessageBox::warning(this, "Contents present",
                                     "Discard?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply != QMessageBox::Yes)
    {
      event->ignore();
      return;
    }
  }

  saveSettings();
  event->accept();
}

void ListModeForm::displayHit(int idx)
{
  ui->tracePlot->clearGraphs();

  if ( (idx < 0) || (idx >= static_cast<int>(hits_.size())))
  {
    ui->tableHitValues->clear();
    ui->tracePlot->replot();
    return;
  }

  Event hit = hits_.at(idx);
  int chan = hit.channel();
  EventModel model;
  if (hitmodels_.count(chan))
    model = hitmodels_.at(chan);

  ui->tableHitValues->setRowCount(hit.value_count());
  ui->tableHitValues->setColumnCount(3);
  ui->tableHitValues->setHorizontalHeaderItem(0, new QTableWidgetItem("Name", QTableWidgetItem::Type));
  ui->tableHitValues->setHorizontalHeaderItem(1, new QTableWidgetItem("Value", QTableWidgetItem::Type));
  ui->tableHitValues->setHorizontalHeaderItem(2, new QTableWidgetItem("Calibrated", QTableWidgetItem::Type));

  for (size_t i = 0; i < hit.value_count(); ++i)
  {
    if (i < model.value_names.size())
      add_to_table(ui->tableHitValues, i, 0, QS(model.value_names.at(i)));
    else
      add_to_table(ui->tableHitValues, i, 0, QString::number(i));
    add_to_table(ui->tableHitValues, i, 1, QString::number(hit.value(i)));
  }

  if (hit.trace_count() &&  hit.trace(0).size())
  {
    uint32_t trace_length = hit.trace(0).size();
    QVector<double> x(trace_length), y(trace_length);
//    int chan = hit.channel();
//    Detector this_det;

    //    if ((chan > -1) && (chan < list_data_->run.detectors.size()))
    //      this_det = list_data_->run.detectors[chan];

//    Calibration this_calibration = this_det.best_calib(16);

    for (std::size_t j=0; j<trace_length; j++)
    {
      x[j] = j;
//      y[j] = this_calibration.transform(hit.trace(0).at(j), 16);
      y[j] = hit.trace(0).at(j);
    }

    ui->tracePlot->addGraph();
    ui->tracePlot->graph(0)->addData(x, y);
    ui->tracePlot->graph(0)->setPen(QPen(Qt::darkGreen));
  }

  ui->tracePlot->xAxis->setLabel("time (ticks)"); //can do better....
  ui->tracePlot->yAxis->setLabel("keV");
  ui->tracePlot->rescaleAxes();
  ui->tracePlot->replot();
}

void ListModeForm::on_pushListStart_clicked()
{
  if (!list_data_.empty())
  {
    int reply = QMessageBox::warning(this, "Contents present",
                                     "Discard?",
                                     QMessageBox::Yes|QMessageBox::Cancel);
    if (reply != QMessageBox::Yes)
      return;
    my_run_ = true;
    list_completed(ListData());
  }

  this->setWindowTitle("List LIVE  \u25b6");

  emit toggleIO(false);
  ui->pushListStop->setEnabled(true);
  my_run_ = true;

  uint64_t duration = ui->timeDuration->total_seconds();

  if (duration == 0)
    return;

  runner_thread_.do_list(interruptor_, duration);
}

void ListModeForm::on_pushListStop_clicked()
{
  ui->pushListStop->setEnabled(false);
  INFO << "List acquisition interrupted by user";
  interruptor_.store(true);
}

void ListModeForm::list_completed(ListData newEvents) {
  if (my_run_) {
    list_data_ = newEvents;

    ui->listSpills->clear();
    for (auto &s : list_data_)
      ui->listSpills->addItem(QString::fromStdString(s->to_string()));

    ui->pushListStop->setEnabled(false);
    this->setWindowTitle("List LIVE");

    emit toggleIO(true);
    my_run_ = false;
  }
}

void ListModeForm::spillSelectionChanged(int row)
{
  this->setCursor(Qt::WaitCursor);
  hits_.clear();
  hitmodels_.clear();
  attr_model_.update(Setting());

  ui->treeAttribs->setVisible(false);
  ui->labelState->setVisible(false);

//  ui->labelEvents->setVisible(false);
//  ui->tableHits->setVisible(false);
//  ui->labelEventVals->setVisible(false);
//  ui->tableHitValues->setVisible(false);

  if ((row >= 0) && (row < static_cast<int>(list_data_.size())))
  {

    for (int i = 0; i < row; i++)
    {
      SpillPtr sp = list_data_.at(i);
      if (!sp)
        continue;
    }


    SpillPtr sp = list_data_.at(row);
    if (sp)
    {
      hits_ = std::vector<Event>(sp->events.begin(), sp->events.end());
      attr_model_.update(sp->state);

      ui->treeAttribs->setVisible(sp->state != Setting());
      ui->labelState->setVisible(sp->state != Setting());

//      ui->labelEvents->setVisible(hits_.size());
//      ui->tableHits->setVisible(hits_.size());
//      ui->labelEventVals->setVisible(hits_.size());
//      ui->tableHitValues->setVisible(hits_.size());
    }
  }


  ui->tableHits->setRowCount(hits_.size());
  ui->tableHits->setColumnCount(2);
  ui->tableHits->setHorizontalHeaderItem(0, new QTableWidgetItem("Time (native)", QTableWidgetItem::Type));
  ui->tableHits->setHorizontalHeaderItem(1, new QTableWidgetItem("Time (ns)", QTableWidgetItem::Type));
  for (size_t i=0; i < hits_.size(); i++)
  {
    Event hit = hits_.at(i);
    int chan = hit.channel();

    add_to_table(ui->tableHits, i, 0, QString::number(hit.timestamp()) );
    if (hitmodels_.count(chan))
      add_to_table(ui->tableHits, i, 1,
                   QS(to_str_decimals(hitmodels_[chan].timebase.to_nanosec(hit.timestamp()), 0)) );
  }



  displayHit(-1);

  this->setCursor(Qt::ArrowCursor);
}

void ListModeForm::event_selection_changed(QItemSelection, QItemSelection)
{
  int idx = -1;
  if (!ui->tableHits->selectionModel()->selectedIndexes().empty())
    idx = ui->tableHits->selectionModel()->selectedIndexes().first().row();
  displayHit(idx);
}
