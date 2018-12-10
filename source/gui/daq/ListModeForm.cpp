#include <gui/daq/ListModeForm.h>
#include "ui_ListModeForm.h"

#include <gui/widgets/qt_util.h>
#include <core/util/lexical_extensions.h>
#include <core/consumer_factory.h>

#include <QSettings>
#include <QMessageBox>

#include <core/util/custom_logger.h>

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

  ui->tableSpills->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->tableSpills->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableSpills->horizontalHeader()->setStretchLastSection(true);
  ui->tableSpills->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(ui->tableSpills->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(spill_selection_changed(QItemSelection,QItemSelection)));


  ui->tableEvents->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->tableEvents->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableEvents->horizontalHeader()->setStretchLastSection(true);
  ui->tableEvents->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(ui->tableEvents->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(event_selection_changed(QItemSelection,QItemSelection)));

  ui->tableValues->setSelectionMode(QAbstractItemView::NoSelection);
  ui->tableValues->horizontalHeader()->setStretchLastSection(true);
  ui->tableValues->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  ui->tableTraces->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->tableTraces->setSelectionBehavior(QAbstractItemView::SelectRows);
  ui->tableTraces->horizontalHeader()->setStretchLastSection(true);
  ui->tableTraces->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  connect(ui->tableTraces->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(trace_selection_changed(QItemSelection,QItemSelection)));

  ui->treeAttribs->setEditTriggers(QAbstractItemView::NoEditTriggers);
  ui->treeAttribs->setModel(&attr_model_);
  ui->treeAttribs->setItemDelegate(&attr_delegate_);
  ui->treeAttribs->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  attr_model_.set_show_address(false);

  ui->timeDuration->set_us_enabled(false);

  ui->treeAttribs->setVisible(false);
  ui->labelState->setVisible(false);

  //  ui->labelEvents->setVisible(false);
  //  ui->tableEvents->setVisible(false);
  //  ui->labelEventVals->setVisible(false);
  //  ui->tableValues->setVisible(false);
}

void ListModeForm::loadSettings()
{
  QSettings settings_;

  settings_.beginGroup("ListDaq");
  ui->timeDuration->set_total_seconds(settings_.value("run_secs", 10).toULongLong());
  settings_.endGroup();
}

void ListModeForm::saveSettings()
{
  QSettings settings_;

  settings_.beginGroup("ListDaq");
  settings_.setValue("run_secs", QVariant::fromValue(ui->timeDuration->total_seconds()));
  settings_.endGroup();
}

void ListModeForm::toggle_push(bool enable, ProducerStatus status, StreamManifest)
{
  bool online = (status & ProducerStatus::can_run);
  ui->pushListStart->setEnabled(enable && online);
  ui->timeDuration->setEnabled(enable && online);
}

ListModeForm::~ListModeForm()
{
  delete ui;
}

void ListModeForm::closeEvent(QCloseEvent *event)
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
    }
    else
    {
      event->ignore();
      return;
    }
  }


  if (!list_data_.empty())
  {
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
  if ( (idx < 0) || (idx >= static_cast<int>(events_.size())))
  {
    ui->tableValues->clear();
    ui->tableTraces->clear();
    return;
  }

  const Event& event = events_.at(idx);

  ui->tableValues->setRowCount(event.value_count());
  ui->tableValues->setColumnCount(3);
  ui->tableValues->setHorizontalHeaderItem(0, new QTableWidgetItem("Name", QTableWidgetItem::Type));
  ui->tableValues->setHorizontalHeaderItem(1, new QTableWidgetItem("Value", QTableWidgetItem::Type));
  ui->tableValues->setHorizontalHeaderItem(2, new QTableWidgetItem("Calibrated", QTableWidgetItem::Type));

  for (size_t i = 0; i < event.value_count(); ++i)
  {
    if (i < event_model_.value_names.size())
      add_to_table(ui->tableValues, i, 0, QS(event_model_.value_names.at(i)));
    else
      add_to_table(ui->tableValues, i, 0, QString::number(i));
    add_to_table(ui->tableValues, i, 1, QString::number(event.value(i)));
    //    add_to_table(ui->tableValues, i, 2, QString::number(event.value(i)));
  }

  ui->tableTraces->setRowCount(event.trace_count());
  ui->tableTraces->setColumnCount(3);
  ui->tableTraces->setHorizontalHeaderItem(0, new QTableWidgetItem("Name", QTableWidgetItem::Type));
  ui->tableTraces->setHorizontalHeaderItem(1, new QTableWidgetItem("Rank", QTableWidgetItem::Type));
  ui->tableTraces->setHorizontalHeaderItem(2, new QTableWidgetItem("Dims", QTableWidgetItem::Type));

  for (size_t i = 0; i < event.trace_count(); ++i)
  {
    if (i < event_model_.trace_names.size())
      add_to_table(ui->tableTraces, i, 0, QS(event_model_.trace_names.at(i)));
    else
      add_to_table(ui->tableTraces, i, 0, QString::number(i));
    add_to_table(ui->tableTraces, i, 1, QString::number(event_model_.traces[i].size()));
    QString dims;
    for (auto d : event_model_.traces[i])
      dims += QString::number(d) + " ";
    add_to_table(ui->tableTraces, i, 2, dims);
  }
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
  INFO("List acquisition interrupted by user");
  interruptor_.store(true);
}

void ListModeForm::list_completed(ListData newEvents)
{
  if (my_run_)
  {
    list_data_ = newEvents;

    ui->tableSpills->setRowCount(list_data_.size());
    ui->tableSpills->setColumnCount(3);
    ui->tableSpills->setHorizontalHeaderItem(0, new QTableWidgetItem("Stream", QTableWidgetItem::Type));
    ui->tableSpills->setHorizontalHeaderItem(1, new QTableWidgetItem("Type", QTableWidgetItem::Type));
    ui->tableSpills->setHorizontalHeaderItem(2, new QTableWidgetItem("Time", QTableWidgetItem::Type));
    size_t i=0;
    for (auto &s : list_data_)
    {
      add_to_table(ui->tableSpills, i, 0,
                   QS(s->stream_id));
      add_to_table(ui->tableSpills, i, 1,
                   QS(Spill::to_str(s->type)));
      add_to_table(ui->tableSpills, i, 2,
                   QS(to_iso_extended(s->time)));
      i++;
    }

    ui->pushListStop->setEnabled(false);
    this->setWindowTitle("List LIVE");

    emit toggleIO(true);
    my_run_ = false;

  }
}

void ListModeForm::spill_selection_changed(QItemSelection, QItemSelection)
{
  int row = -1;
  if (!ui->tableSpills->selectionModel()->selectedIndexes().empty())
    row = ui->tableSpills->selectionModel()->selectedIndexes().first().row();

  this->setCursor(Qt::WaitCursor);
  events_.clear();
  attr_model_.update(Setting());

  ui->treeAttribs->setVisible(false);
  ui->labelState->setVisible(false);

  //  ui->labelEvents->setVisible(false);
  //  ui->tableEvents->setVisible(false);
  //  ui->labelEventVals->setVisible(false);
  //  ui->tableValues->setVisible(false);

  if ((row >= 0) && (row < static_cast<int>(list_data_.size())))
  {
    SpillPtr sp = list_data_.at(row);
    if (sp)
    {
      events_ = std::vector<Event>(sp->events.begin(), sp->events.end());
      attr_model_.update(sp->state);

      ui->treeAttribs->setVisible(sp->state != Setting());
      ui->labelState->setVisible(sp->state != Setting());
      event_model_ = sp->event_model;

//      DBG( "Received event model " << event_model_;

      //      ui->labelEvents->setVisible(events_.size());
      //      ui->tableEvents->setVisible(events_.size());
      //      ui->labelEventVals->setVisible(events_.size());
      //      ui->tableValues->setVisible(events_.size());
    }
  }

  ui->tableEvents->setRowCount(events_.size());
  ui->tableEvents->setColumnCount(2);
  ui->tableEvents->setHorizontalHeaderItem(0, new QTableWidgetItem("Time (native)", QTableWidgetItem::Type));
  ui->tableEvents->setHorizontalHeaderItem(1, new QTableWidgetItem("Time (ns)", QTableWidgetItem::Type));
  for (size_t i=0; i < events_.size(); i++)
  {
    const Event& event = events_.at(i);

    add_to_table(ui->tableEvents, i, 0,
                 QString::number(event.timestamp()) );
    add_to_table(ui->tableEvents, i, 1, QS(event_model_.timebase.debug())
                 + " = " +
                 QS(std::to_string(event_model_.timebase.to_nanosec(event.timestamp()))));
//                 QS(to_str_decimals(event_model_.timebase.to_nanosec(event.timestamp()), 3));
  }

  event_selection_changed(QItemSelection(), QItemSelection());

  this->setCursor(Qt::ArrowCursor);
}

void ListModeForm::event_selection_changed(QItemSelection, QItemSelection)
{
  int idx = -1;
  if (!ui->tableEvents->selectionModel()->selectedIndexes().empty())
    idx = ui->tableEvents->selectionModel()->selectedIndexes().first().row();
  displayHit(idx);
  trace_selection_changed(QItemSelection(), QItemSelection());
}

void ListModeForm::trace_selection_changed(QItemSelection, QItemSelection)
{
  ConsumerMetadata md = ConsumerFactory::singleton().create_prototype("Prebinned 1D");
  if (!ui->tableEvents->selectionModel()->selectedIndexes().empty() &&
      !ui->tableTraces->selectionModel()->selectedIndexes().empty())
  {
    int spill_i = ui->tableSpills->selectionModel()->selectedIndexes().first().row();
    int event_i = ui->tableEvents->selectionModel()->selectedIndexes().first().row();
    int trace_i = ui->tableTraces->selectionModel()->selectedIndexes().first().row();

    SpillPtr sp = list_data_.at(spill_i);
    md.set_attribute(Setting::text("stream_id", sp->stream_id));
    md.set_attribute(Setting::text("trace_id", event_model_.trace_names.at(trace_i)));
    md.set_attribute(Setting::text("appearance", QColor(Qt::darkRed).name().toStdString()));
    trace_ = ConsumerFactory::singleton().create_from_prototype(md);

    SpillPtr sp3 = std::make_shared<Spill>(sp->stream_id, Spill::Type::running);
    sp3->event_model = sp->event_model;
    sp3->events.reserve(1, sp->event_model);
    sp3->events.last() = events_[event_i];
    ++sp3->events;
    sp3->events.finalize();
    trace_->push_spill(*sp3);
  }
  else
  {
    trace_ = ConsumerFactory::singleton().create_from_prototype(md);
  }

  ui->tracePlot->setConsumer(trace_);
  ui->tracePlot->update();
  ui->tracePlot->refresh();
}
