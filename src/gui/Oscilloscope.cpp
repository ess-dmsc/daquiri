#include "Oscilloscope.h"
#include "ui_Oscilloscope.h"
#include "QHist.h"
#include <QSettings>


Oscilloscope::Oscilloscope(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::Oscilloscope)
{
  ui->setupUi(this);

  ui->widgetPlot->setScaleType("Linear");
  ui->widgetPlot->setPlotStyle("Step center");
  ui->widgetPlot->setShowMarkerLabels(false);
  ui->widgetPlot->setVisibleOptions(QPlot::zoom |
                                    QPlot::title |
                                    QPlot::save);
  ui->widgetPlot->setTitle("");
  ui->widgetPlot->setAxisLabels("time (ticks)", "energy/channel");

  connect(ui->selectorChannels, SIGNAL(itemSelected(SelectorItem)), this, SLOT(channelDetails(SelectorItem)));
  connect(ui->selectorChannels, SIGNAL(itemToggled(SelectorItem)), this, SLOT(channelToggled(SelectorItem)));
}

Oscilloscope::~Oscilloscope()
{
  delete ui;
}

void Oscilloscope::closeEvent(QCloseEvent *event)
{
  event->accept();
}

void Oscilloscope::updateMenu(std::vector<DAQuiri::Detector> dets)
{
  channels_ = dets;

  QVector<SelectorItem> my_channels = ui->selectorChannels->items();

  bool changed = false;

  if (static_cast<int>(dets.size()) < my_channels.size())
  {
    my_channels.resize(dets.size());
    changed = true;
  }

  ui->pushShowAll->setEnabled(!dets.empty());
  ui->pushHideAll->setEnabled(!dets.empty());

  QVector<QColor> palette {Qt::darkCyan, Qt::darkBlue, Qt::darkGreen, Qt::darkRed, Qt::darkYellow, Qt::darkMagenta, Qt::red, Qt::blue};

  for (size_t i=0; i < dets.size(); ++i) {
    if (static_cast<int>(i) >= my_channels.size()) {
      my_channels.push_back(SelectorItem());
      my_channels[i].visible = true;
      changed = true;
    }

    if (my_channels[i].text != QString::fromStdString(dets[i].id()))
    {
      my_channels[i].data = QVariant::fromValue(i);
      my_channels[i].text = QString::fromStdString(dets[i].id());
      my_channels[i].color = palette[i % palette.size()];
      changed = true;
    }
  }

  if (changed) {
    ui->selectorChannels->setItems(my_channels);
    replot();
  }
}

void Oscilloscope::channelToggled(SelectorItem)
{
  replot();
}

void Oscilloscope::channelDetails(SelectorItem item)
{
  int i = item.data.toInt();
  QString text;
  if ((i > -1) && (i < static_cast<int32_t>(traces_.size())))
  {
    Detector det = channels_.at(i);
    text += QString::fromStdString(det.id());
    text += " (" + QString::fromStdString(det.type()) + ")";
  }
  ui->widgetPlot->setTitle(text);
}

void Oscilloscope::toggle_push(bool enable, DAQuiri::ProducerStatus status)
{
  bool online = (status & ProducerStatus::can_oscil);
  ui->pushOscilRefresh->setEnabled(enable && online);
}

void Oscilloscope::on_pushOscilRefresh_clicked()
{
  emit refresh_oscil();
}

void Oscilloscope::oscil_complete(DAQuiri::OscilData traces)
{
  if (!this->isVisible())
    return;

  traces_ = traces;

//  updateMenu(dets);

  replot();
}

void Oscilloscope::replot()
{
  ui->widgetPlot->clearAll();

  QString unit = "n/a";

  if (!traces_.empty())
  {
    QVector<SelectorItem> my_channels = ui->selectorChannels->items();

    for (size_t i=0; i < traces_.size(); i++)
    {
      Event trace = traces_.at(i);
      if (!trace.trace(0).size())
        continue;

      QPlot::HistMap1D hist;
      for (size_t j=0; j < trace.trace(0).size(); ++j)
        hist[trace.timestamp().nanosecs() * 0.001] = trace.trace(0).at(j);

      if ((static_cast<int>(i) < my_channels.size()) && (my_channels[i].visible))
        ui->widgetPlot->addGraph(hist, QPen(my_channels[i].color, 1));
    }
  }

  ui->widgetPlot->setAxisLabels("time (\u03BCs)", "energy (" + unit + ")");
  ui->widgetPlot->replotAll();
  ui->widgetPlot->tightenX();
}

void Oscilloscope::on_pushShowAll_clicked()
{
  ui->selectorChannels->show_all();
  replot();
}

void Oscilloscope::on_pushHideAll_clicked()
{
  ui->selectorChannels->hide_all();
  replot();
}
