#include "form_plot1d.h"
#include "ui_form_plot1d.h"
#include "ConsumerDialog.h"
#include "custom_timer.h"
#include "boost/algorithm/string.hpp"
#include "QHist.h"
#include "qt_util.h"

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
  mySpectra = new_set;
  updateUI();
}

void FormPlot1D::spectrumLooksChanged(SelectorItem item)
{
  SinkPtr someSpectrum = mySpectra->get_sink(item.data.toLongLong());
  if (someSpectrum)
    someSpectrum->set_attribute(Setting::boolean("visible", item.visible));
  reset_content();
  mySpectra->activate();
}

void FormPlot1D::spectrumDoubleclicked(SelectorItem /*item*/)
{
  on_pushFullInfo_clicked();
}

void FormPlot1D::spectrumDetails(SelectorItem /*item*/)
{
  SelectorItem itm = spectraSelector->selected();

  SinkPtr someSpectrum = mySpectra->get_sink(itm.data.toLongLong());

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

  Setting tothits = md.get_attribute("total_hits");
  double rate_total = 0;
  if (live > 0)
    rate_total = tothits.get_number() / live; // total count rate corrected for dead time

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
      "<nobr>Count: " + QString::number(tothits.get_number()) + "</nobr><br/>"
      "<nobr>Rate (inst/total): " + QString::number(rate_inst) + "cps / " + QString::number(rate_total) + "cps</nobr><br/>"
      "<nobr>Live / real:  " + QString::number(live) + "s / " + QString::number(real) + "s</nobr><br/>"
      "<nobr>Dead:  " + QString::number(dead) + "%</nobr><br/>";

  ui->labelSpectrumInfo->setText(infoText);
  ui->pushFullInfo->setEnabled(true);
}

void FormPlot1D::reset_content()
{
  nonempty_ = false;
  ui->mcaPlot->clearAll();
  ui->mcaPlot->replotExtras();
  ui->mcaPlot->tightenX();
  ui->mcaPlot->replot();
}

void FormPlot1D::update_plot()
{
  this->setCursor(Qt::WaitCursor);
  //  CustomTimer guiside(true);

  int numvisible {0};

  ui->mcaPlot->clearPrimary();
  for (auto &q: mySpectra->get_sinks(1))
  {
    if (!q.second)
      continue;

    ConsumerMetadata md = q.second->metadata();

    if (!md.get_attribute("visible").triggered())
      continue;

    double rescale  = md.get_attribute("rescale").get_number();

    QVector<double> x = QVector<double>::fromStdVector(q.second->axis_values(0));

    std::shared_ptr<EntryList> spectrum_data =
        std::move(q.second->data_range({{0, x.size()}}));

    QPlot::HistMap1D hist;
    for (auto it : *spectrum_data)
    {
      double xx = x[it.first[0]];
      double yy = to_double( it.second ) * rescale;
      hist[xx] = yy;
    }

    if (hist.empty())
      continue;

    numvisible++;

    auto pen = QPen(QColor(QString::fromStdString(md.get_attribute("appearance").get_text())), 1);
    ui->mcaPlot->addGraph(hist, pen);
  }

  replot_markers();

  std::string new_label = boost::algorithm::trim_copy(mySpectra->identity());
  ui->mcaPlot->setTitle(QString::fromStdString(new_label));

  spectrumDetails(SelectorItem());

  if (!nonempty_ && (numvisible > 0))
  {
    ui->mcaPlot->zoomOut();
    nonempty_ = (numvisible > 0);
  }

  //  DBG << "<Plot1D> plotting took " << guiside.ms() << " ms";
  this->setCursor(Qt::ArrowCursor);
}

void FormPlot1D::on_pushFullInfo_clicked()
{  
  SinkPtr someSpectrum = mySpectra->get_sink(spectraSelector->selected().data.toLongLong());
  if (!someSpectrum)
    return;

  DialogSpectrum* newSpecDia = new DialogSpectrum(someSpectrum->metadata(), std::vector<Detector>(), detectors_, true, false, this);
  connect(newSpecDia, SIGNAL(delete_spectrum()), this, SLOT(spectrumDetailsDelete()));
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
  mySpectra->delete_sink(spectraSelector->selected().data.toLongLong());

  updateUI();
}

void FormPlot1D::updateUI()
{
  SelectorItem chosen = spectraSelector->selected();
  QVector<SelectorItem> items;
  QSet<QString> dets;

  for (auto &q : mySpectra->get_sinks(1))
  {
    ConsumerMetadata md;
    if (q.second != nullptr)
      md = q.second->metadata();

    if (!md.detectors.empty())
      dets.insert(QString::fromStdString(md.detectors.front().name()));

    SelectorItem new_spectrum;
    new_spectrum.text = QString::fromStdString(md.get_attribute("name").get_text());
    new_spectrum.data = QVariant::fromValue(q.first);
    new_spectrum.color = QColor(QString::fromStdString(md.get_attribute("appearance").get_text()));
    new_spectrum.visible = md.get_attribute("visible").triggered();

    items.push_back(new_spectrum);
  }


  menuEffCal.clear();

  for (auto &q : dets)
    menuEffCal.addAction(q);

  spectraSelector->setItems(items);
  spectraSelector->setSelected(chosen.text);

  ui->scrollArea->updateGeometry();

  ui->toolColors->setEnabled(spectraSelector->items().size());
  ui->toolDelete->setEnabled(spectraSelector->items().size());

  mySpectra->activate();
}

void FormPlot1D::replot_markers()
{
  QList<QPlot::Marker1D> markers;

  ui->mcaPlot->setMarkers(markers);
  ui->mcaPlot->replotExtras();
  ui->mcaPlot->replot();
}


void FormPlot1D::showAll()
{
  spectraSelector->show_all();
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items)
  {
    SinkPtr someSpectrum = mySpectra->get_sink(q.data.toLongLong());
    if (someSpectrum)
      someSpectrum->set_attribute(Setting::boolean("visible", true));
  }
  mySpectra->activate();
}

void FormPlot1D::hideAll()
{
  spectraSelector->hide_all();
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items)
  {
    SinkPtr someSpectrum = mySpectra->get_sink(q.data.toLongLong());
    if (someSpectrum)
      someSpectrum->set_attribute(Setting::boolean("visible", false));
  }
  mySpectra->activate();
}

void FormPlot1D::randAll()
{
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items)
  {
    SinkPtr someSpectrum = mySpectra->get_sink(q.data.toLongLong());
    if (!someSpectrum)
      continue;
    auto col = generateColor().name(QColor::HexArgb).toStdString();
    someSpectrum->set_attribute(Setting::color("appearance", col));
  }
  updateUI();
}

void FormPlot1D::set_scale_type(QString sct)
{
  ui->mcaPlot->setScaleType(sct);
}

void FormPlot1D::set_plot_style(QString stl)
{
  ui->mcaPlot->setPlotStyle(stl);
}

QString FormPlot1D::scale_type()
{
  return ui->mcaPlot->scaleType();
}

QString FormPlot1D::plot_style()
{
  return ui->mcaPlot->plotStyle();
}

void FormPlot1D::deleteSelected()
{
  mySpectra->delete_sink(spectraSelector->selected().data.toLongLong());
  updateUI();
}

void FormPlot1D::deleteShown()
{
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items)
    if (q.visible)
      mySpectra->delete_sink(q.data.toLongLong());
  updateUI();
}

void FormPlot1D::deleteHidden()
{
  QVector<SelectorItem> items = spectraSelector->items();
  for (auto &q : items)
    if (!q.visible)
      mySpectra->delete_sink(q.data.toLongLong());
  updateUI();
}
