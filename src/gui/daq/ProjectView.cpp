#include "ProjectView.h"
#include "ui_ProjectView.h"
#include "ConsumerDialog.h"
#include "custom_timer.h"
#include "boost/algorithm/string.hpp"
#include "QHist.h"
#include "qt_util.h"
#include "Consumer1D.h"
#include "Consumer2D.h"

using namespace DAQuiri;

ProjectView::ProjectView(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::ProjectView)
{
  ui->setupUi(this);

  selector_ = new SelectorWidget();
  selector_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  selector_->setMaximumWidth(280);
  ui->scrollArea->setWidget(selector_);
  //ui->scrollArea->viewport()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  //ui->scrollArea->viewport()->setMinimumWidth(283);

  connect(selector_, SIGNAL(itemSelected(SelectorItem)), this, SLOT(selectorItemSelected(SelectorItem)));
  connect(selector_, SIGNAL(itemToggled(SelectorItem)), this, SLOT(selectorItemToggled(SelectorItem)));
  connect(selector_, SIGNAL(itemDoubleclicked(SelectorItem)), this, SLOT(selectorItemDoubleclicked(SelectorItem)));

  colors_menu_.addAction(QIcon(":/icons/show16.png"), "Show all",
                         this, SLOT(showAll()));
  colors_menu_.addAction(QIcon(":/icons/hide16.png"), "Hide all",
                         this, SLOT(hideAll()));
  colors_menu_.addAction(QIcon(":/icons/oxy/16/roll.png"), "Randomize colors",
                         this, SLOT(randAll()));
  ui->toolColors->setMenu(&colors_menu_);

  delete_menu_.addAction(QIcon(":/icons/oxy/16/editdelete.png"), "Delete selected",
                         this, SLOT(deleteSelected()));
  delete_menu_.addAction(QIcon(":/icons/show16.png"), "Delete shown",
                         this, SLOT(deleteShown()));
  delete_menu_.addAction(QIcon(":/icons/hide16.png"), "Delete hidden",
                         this, SLOT(deleteHidden()));
  ui->toolDelete->setMenu(&delete_menu_);

  tile_menu_.addAction(QIcon(":/icons/oxy/16/editdelete.png"), "Tile grid",
                       this, SLOT(tile_grid()));
  tile_menu_.addAction(QIcon(":/icons/show16.png"), "Tile horizontal",
                       this, SLOT(tile_horizontal()));
  tile_menu_.addAction(QIcon(":/icons/hide16.png"), "Tile vertical",
                       this, SLOT(tile_vertical()));
  ui->toolTile->setMenu(&tile_menu_);
}

ProjectView::~ProjectView()
{
  delete ui;
}

void ProjectView::setSpectra(ProjectPtr new_set)
{
  consumers_.clear();
  ui->area->closeAllSubWindows();
  project_ = new_set;
  updateUI();
  enforce_all();
  update();
}

void ProjectView::selectorItemToggled(SelectorItem item)
{
  enforce_all();
  update_plots();
}

void ProjectView::enforce_item(SelectorItem item)
{
  int64_t id = item.data.toLongLong();
  ConsumerPtr consumer = project_->get_sink(id);
  if (!consumer)
    return;
  consumer->set_attribute(Setting::boolean("visible", item.visible));
  if (consumers_.count(id) && !item.visible)
  {
    consumers_[id]->parentWidget()->close();
    consumers_.remove(id);
  }
  else if (!consumers_.count(id) && item.visible)
  {
    AbstractConsumerWidget* widget;
    if (consumer->dimensions() == 1)
      widget = new Consumer1D();
    else if (consumer->dimensions() == 2)
      widget = new Consumer2D();
    else
      return;

    widget->setAttribute(Qt::WA_DeleteOnClose);
    connect( widget, SIGNAL(destroyed(QObject*)),
             this, SLOT(consumerWidgetDestroyed(QObject*)) );
    ui->area->addSubWindow(widget);
    consumers_[id] = widget;
    widget->show();
    widget->setConsumer(consumer);
  }
}


void ProjectView::selectorItemDoubleclicked(SelectorItem /*item*/)
{
  on_pushFulINFO_clicked();
}

void ProjectView::selectorItemSelected(SelectorItem /*item*/)
{
  SelectorItem itm = selector_->selected();

  ConsumerPtr consumer = project_->get_sink(itm.data.toLongLong());

  if (!consumer)
  {
    ui->labelSpectrumInfo->setText("<html><head/><body><p>Left-click: see statistics here<br/>Right click: toggle visibility<br/>Double click: details / analysis</p></body></html>");
    ui->pushFulINFO->setEnabled(false);
    return;
  }

  if (consumers_.count(itm.data.toLongLong()))
    consumers_[itm.data.toLongLong()]->raise();

  ConsumerMetadata md = consumer->metadata();

  double real = md.get_attribute("real_time").duration().total_milliseconds() * 0.001;
  double live = md.get_attribute("live_time").duration().total_milliseconds() * 0.001;
  double total_count = md.get_attribute("total_count").get_number();
  double rate_total = 0;
  if (live > 0)
    rate_total = total_count / live;
  double dead = 100;
  if (real > 0)
    dead = (real - live) * 100.0 / real;
  double rate_inst = md.get_attribute("instant_rate").get_number();

  Detector det = Detector();
  if (!md.detectors.empty())
    det = md.detectors[0];

  uint16_t bits = md.get_attribute("resolution").selection();

  QString detstr = "Detector: " + QString::fromStdString(det.id());

  QString infoText =
      "<nobr>" + itm.text + "(" + QString::fromStdString(consumer->type())
      + ", " + QString::number(bits) + "bits)</nobr><br/>"
      "<nobr>" + detstr + "</nobr><br/>"
      "<nobr>Count: " + QString::number(total_count) + "</nobr><br/>"
      "<nobr>Rate (inst/total): " + QString::number(rate_inst) + "cps / " + QString::number(rate_total) + "cps</nobr><br/>"
      "<nobr>Live / real:  " + QString::number(live) + "s / " + QString::number(real) + "s</nobr><br/>"
      "<nobr>Dead:  " + QString::number(dead) + "%</nobr><br/>";

  ui->labelSpectrumInfo->setText(infoText);
  ui->pushFulINFO->setEnabled(true);
}

void ProjectView::updateUI()
{
  SelectorItem chosen = selector_->selected();
  QVector<SelectorItem> items;

  for (auto &q : project_->get_sinks())
  {
    if (!q.second)
      continue;

    ConsumerMetadata md = q.second->metadata();

    Setting appearance = md.get_attribute("appearance");
    QColor color;
    if (appearance != Setting())
      color = QColor(QString::fromStdString(appearance.get_text()));
    else
      color = Qt::black;

    SelectorItem consumer_item;
    consumer_item.text = QString::fromStdString(md.get_attribute("name").get_text());
    consumer_item.data = QVariant::fromValue(q.first);
    consumer_item.color = color;
    consumer_item.visible = md.get_attribute("visible").triggered();

    items.push_back(consumer_item);
  }

  selector_->setItems(items);
  selector_->setSelected(chosen.text);

  ui->scrollArea->updateGeometry();

  ui->toolColors->setEnabled(selector_->items().size());
  ui->toolDelete->setEnabled(selector_->items().size());
}

void ProjectView::enforce_all()
{
  size_t initial = consumers_.size();
  for (auto item : selector_->items())
    enforce_item(item);
  if (consumers_.size() != initial)
    tile_grid();
}

void ProjectView::update_plots()
{
  this->setCursor(Qt::WaitCursor);
  //  CustomTimer guiside(true);

  for (auto &consumer_widget : consumers_)
    consumer_widget->update();

  selectorItemSelected(SelectorItem());

  //  DBG << "<Plot1D> plotting took " << guiside.ms() << " ms";
  this->setCursor(Qt::ArrowCursor);
}

void ProjectView::on_pushFulINFO_clicked()
{  
  ConsumerPtr consumer = project_->get_sink(selector_->selected().data.toLongLong());
  if (!consumer)
    return;

  ConsumerDialog* newSpecDia =
      new ConsumerDialog(consumer->metadata(), std::vector<Detector>(),
                         detectors_, true, false, this);
  if (newSpecDia->exec() == QDialog::Accepted)
  {
    ConsumerMetadata md = newSpecDia->product();
    consumer->set_detectors(md.detectors);
    consumer->set_attributes(md.attributes());
    updateUI();
    enforce_all();
    update();
  }
}

void ProjectView::showAll()
{
  selector_->show_all();
  for (auto &q : project_->get_sinks())
    if (q.second)
      q.second->set_attribute(Setting::boolean("visible", true));
  updateUI();
  enforce_all();
  update();
}

void ProjectView::hideAll()
{
  selector_->hide_all();
  for (auto &q : project_->get_sinks())
    if (q.second)
      q.second->set_attribute(Setting::boolean("visible", false));
  updateUI();
  enforce_all();
  update();
}

void ProjectView::randAll()
{
  for (auto &q : project_->get_sinks())
    for (auto &q : project_->get_sinks())
      if (q.second)
      {
        auto col = generateColor().name(QColor::HexArgb).toStdString();
        q.second->set_attribute(Setting::color("appearance", col));
      }
  updateUI();
  update();
}

void ProjectView::deleteSelected()
{
  project_->delete_sink(selector_->selected().data.toLongLong());
  updateUI();
  enforce_all();
  update();
}

void ProjectView::deleteShown()
{
  for (auto &q : selector_->items())
    if (q.visible)
      project_->delete_sink(q.data.toLongLong());
  updateUI();
  enforce_all();
  update();
}

void ProjectView::deleteHidden()
{
  for (auto &q : selector_->items())
    if (!q.visible)
      project_->delete_sink(q.data.toLongLong());
  updateUI();
  enforce_all();
  update();
}

void ProjectView::tile_grid()
{
  ui->area->tileSubWindows();
}

void ProjectView::tile_horizontal()
{
  if (ui->area->subWindowList().isEmpty())
    return;

  QPoint position(0, 0);

  foreach (QMdiSubWindow *window, ui->area->subWindowList())
  {
    QRect rect(0, 0, ui->area->width() / ui->area->subWindowList().count(),
               ui->area->height());
    window->setGeometry(rect);
    window->move(position);
    position.setX(position.x() + window->width());
  }
}

void ProjectView::tile_vertical()
{
  if (ui->area->subWindowList().isEmpty())
    return;

  QPoint position(0, 0);

  foreach (QMdiSubWindow *window, ui->area->subWindowList())
  {
    QRect rect(0, 0, ui->area->width(),
               ui->area->height() / ui->area->subWindowList().count());
    window->setGeometry(rect);
    window->move(position);
    position.setY(position.y() + window->height());
  }
}

void ProjectView::consumerWidgetDestroyed(QObject* o)
{
  for (auto it = consumers_.begin(); it != consumers_.end();)
    if (it.value() == o)
    {
      auto c = project_->get_sink(it.key());
      if (c)
        c->set_attribute(Setting::boolean("visible", false));
      consumers_.erase(it++);
    }
    else
      ++it;
  updateUI();
}

