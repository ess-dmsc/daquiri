#include "form_plot1d.h"
#include "ui_form_plot1d.h"
#include "ConsumerDialog.h"
#include "custom_timer.h"
#include "boost/algorithm/string.hpp"
#include "QHist.h"
#include "qt_util.h"
#include "Consumer1D.h"
#include "Consumer2D.h"

using namespace DAQuiri;

FormPlot1D::FormPlot1D(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::FormPlot1D)
{
  ui->setupUi(this);

  spectraSelector = new SelectorWidget();
  spectraSelector->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  spectraSelector->setMaximumWidth(280);
  ui->scrollArea->setWidget(spectraSelector);
  //ui->scrollArea->viewport()->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  //ui->scrollArea->viewport()->setMinimumWidth(283);

  connect(spectraSelector, SIGNAL(itemSelected(SelectorItem)), this, SLOT(spectrumDetails(SelectorItem)));
  connect(spectraSelector, SIGNAL(itemToggled(SelectorItem)), this, SLOT(spectrumLooksChanged(SelectorItem)));
  connect(spectraSelector, SIGNAL(itemDoubleclicked(SelectorItem)), this, SLOT(spectrumDoubleclicked(SelectorItem)));

  menuColors.addAction(QIcon(":/icons/show16.png"), "Show all", this, SLOT(showAll()));
  menuColors.addAction(QIcon(":/icons/hide16.png"), "Hide all", this, SLOT(hideAll()));
  menuColors.addAction(QIcon(":/icons/oxy/16/roll.png"), "Randomize all colors", this, SLOT(randAll()));
  ui->toolColors->setMenu(&menuColors);

  menuDelete.addAction(QIcon(":/icons/oxy/16/editdelete.png"), "Delete selected spectrum", this, SLOT(deleteSelected()));
  menuDelete.addAction(QIcon(":/icons/show16.png"), "Delete shown spectra", this, SLOT(deleteShown()));
  menuDelete.addAction(QIcon(":/icons/hide16.png"), "Delete hidden spectra", this, SLOT(deleteHidden()));
  ui->toolDelete->setMenu(&menuDelete);
}

FormPlot1D::~FormPlot1D()
{
  delete ui;
}

void FormPlot1D::setSpectra(ProjectPtr new_set)
{
  reset_content();
  project_ = new_set;

//  ui->Plot2d->setSpectra(project_);
//  ui->Plot2d->update_plot(true);

  updateUI();
}

void FormPlot1D::spectrumLooksChanged(SelectorItem item)
{
  ConsumerPtr someSpectrum = project_->get_sink(item.data.toLongLong());
  if (someSpectrum)
    someSpectrum->set_attribute(Setting::boolean("visible", item.visible));
  reset_content();
  project_->activate();
}

void FormPlot1D::spectrumDoubleclicked(SelectorItem /*item*/)
{
  on_pushFullInfo_clicked();
}

void FormPlot1D::spectrumDetails(SelectorItem /*item*/)
{
  SelectorItem itm = spectraSelector->selected();

  ConsumerPtr someSpectrum = project_->get_sink(itm.data.toLongLong());

  ConsumerMetadata md;

  if (someSpectrum)
    md = someSpectrum->metadata();
  else
  {
    ui->labelSpectrumInfo->setText("<html><head/><body><p>Left-click: see statistics here<br/>Right click: toggle visibility<br/>Double click: details / analysis</p></body></html>");
    ui->pushFullInfo->setEnabled(false);
    return;
  }

  std::string type = someSpectrum->type();
  double real = md.get_attribute("real_time").duration().total_milliseconds() * 0.001;
  double live = md.get_attribute("live_time").duration().total_milliseconds() * 0.001;

  double total_count = md.get_attribute("total_count").get_number();
  double rate_total = 0;
  if (live > 0)
    rate_total = total_count / live; // total count rate corrected for dead time

  double dead = 100;
  if (real > 0)
    dead = (real - live) * 100.0 / real;

  double rate_inst = md.get_attribute("instant_rate").get_number();

  Detector det = Detector();
  if (!md.detectors.empty())
    det = md.detectors[0];

  uint16_t bits = md.get_attribute("resolution").selection();

  QString detstr("Detector: ");
  detstr += QString::fromStdString(det.name());

  QString infoText =
      "<nobr>" + itm.text + "(" + QString::fromStdString(type) + ", " + QString::number(bits) + "bits)</nobr><br/>"
      "<nobr>" + detstr + "</nobr><br/>"
      "<nobr>Count: " + QString::number(total_count) + "</nobr><br/>"
      "<nobr>Rate (inst/total): " + QString::number(rate_inst) + "cps / " + QString::number(rate_total) + "cps</nobr><br/>"
      "<nobr>Live / real:  " + QString::number(live) + "s / " + QString::number(real) + "s</nobr><br/>"
      "<nobr>Dead:  " + QString::number(dead) + "%</nobr><br/>";

  ui->labelSpectrumInfo->setText(infoText);
  ui->pushFullInfo->setEnabled(true);
}

void FormPlot1D::reset_content()
{
  nonempty_ = false;

  spectra_.clear();
  ui->area->closeAllSubWindows();

//  ui->Plot2d->reset_content(); //is this necessary?

//  ui->mcaPlot->clearAll();
//  ui->mcaPlot->replotExtras();
//  ui->mcaPlot->tightenX();
//  ui->mcaPlot->replot();
}

void FormPlot1D::update_plot()
{
  this->setCursor(Qt::WaitCursor);
  //  CustomTimer guiside(true);

  int numvisible {0};

  for (auto &q : spectraSelector->items())
  {
    int64_t id = q.data.toLongLong();
    ConsumerPtr cons = project_->get_sink(id);

    if (!cons)
      continue;

    if (!q.visible)
      continue;

    if (spectra_.count(id))
      spectra_[id]->update();
    else
    {
      AbstractConsumerWidget* spectrum;
      if (cons->dimensions() == 1)
        spectrum = new Consumer1D();
      else if (cons->dimensions() == 2)
        spectrum = new Consumer2D();
      else
        continue;

      spectrum->setAttribute(Qt::WA_DeleteOnClose);
      ui->area->addSubWindow(spectrum);
      spectra_[id] = spectrum;
      spectrum->show();
      spectrum->setConsumer(cons);
    }

    numvisible++;
  }

  spectrumDetails(SelectorItem());

  if (!nonempty_ && (numvisible > 0))
  {
//    ui->mcaPlot->zoomOut();
    nonempty_ = (numvisible > 0);
  }

  //  DBG << "<Plot1D> plotting took " << guiside.ms() << " ms";
  this->setCursor(Qt::ArrowCursor);
}

void FormPlot1D::on_pushFullInfo_clicked()
{  
  ConsumerPtr someSpectrum = project_->get_sink(spectraSelector->selected().data.toLongLong());
  if (!someSpectrum)
    return;

  ConsumerDialog* newSpecDia =
      new ConsumerDialog(someSpectrum->metadata(), std::vector<Detector>(),
                         detectors_, true, false, this);
  if (newSpecDia->exec() == QDialog::Accepted)
  {
    ConsumerMetadata md = newSpecDia->product();
    someSpectrum->set_detectors(md.detectors);
    someSpectrum->set_attributes(md.attributes());
    updateUI();
  }
}

void FormPlot1D::spectrumDetailsDelete()
{
  project_->delete_sink(spectraSelector->selected().data.toLongLong());

  updateUI();
}

void FormPlot1D::updateUI()
{
  SelectorItem chosen = spectraSelector->selected();
  QVector<SelectorItem> items;

  for (auto &q : project_->get_sinks())
  {
    if (!q.second ||
        (q.second->dimensions() < 1) ||
        (q.second->dimensions() > 2))
      continue;

    ConsumerMetadata md = q.second->metadata();

    SelectorItem new_spectrum;
    new_spectrum.text = QString::fromStdString(md.get_attribute("name").get_text());
    new_spectrum.data = QVariant::fromValue(q.first);
    new_spectrum.color = QColor(QString::fromStdString(md.get_attribute("appearance").get_text()));
    new_spectrum.visible = md.get_attribute("visible").triggered();

    items.push_back(new_spectrum);

//    ui->Plot2d->updateUI();
  }

  spectraSelector->setItems(items);
  spectraSelector->setSelected(chosen.text);

  ui->scrollArea->updateGeometry();

  ui->toolColors->setEnabled(spectraSelector->items().size());
  ui->toolDelete->setEnabled(spectraSelector->items().size());

  project_->activate();
}

void FormPlot1D::showAll()
{
  spectraSelector->show_all();
  for (auto &q : spectraSelector->items())
  {
    ConsumerPtr someSpectrum = project_->get_sink(q.data.toLongLong());
    if (someSpectrum)
      someSpectrum->set_attribute(Setting::boolean("visible", true));
  }
  project_->activate();
}

void FormPlot1D::hideAll()
{
  spectraSelector->hide_all();
  for (auto &q : spectraSelector->items())
  {
    ConsumerPtr someSpectrum = project_->get_sink(q.data.toLongLong());
    if (someSpectrum)
      someSpectrum->set_attribute(Setting::boolean("visible", false));
  }
  project_->activate();
}

void FormPlot1D::randAll()
{
  for (auto &q : spectraSelector->items())
  {
    ConsumerPtr someSpectrum = project_->get_sink(q.data.toLongLong());
    if (!someSpectrum)
      continue;
    auto col = generateColor().name(QColor::HexArgb).toStdString();
    someSpectrum->set_attribute(Setting::color("appearance", col));
  }
  updateUI();
}

void FormPlot1D::deleteSelected()
{
  project_->delete_sink(spectraSelector->selected().data.toLongLong());
  updateUI();
}

void FormPlot1D::deleteShown()
{
  for (auto &q : spectraSelector->items())
    if (q.visible)
      project_->delete_sink(q.data.toLongLong());
  updateUI();
}

void FormPlot1D::deleteHidden()
{
  for (auto &q : spectraSelector->items())
    if (!q.visible)
      project_->delete_sink(q.data.toLongLong());
  updateUI();
}

void FormPlot1D::on_pushTile_clicked()
{
  ui->area->tileSubWindows();
}
