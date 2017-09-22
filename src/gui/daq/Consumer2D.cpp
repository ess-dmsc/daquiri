#include "Consumer2D.h"
#include "custom_timer.h"

#include "custom_logger.h"

using namespace DAQuiri;

Consumer2D::Consumer2D(QWidget *parent)
  : AbstractConsumerWidget(parent)
  , plot_ (new QPlot::Plot2D())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(plot_);
  plot_->setSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::MinimumExpanding);
  plot_->setGradient("Spectrum2");
  plot_->setShowGradientLegend(true);
  connect(plot_, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel(QWheelEvent*)));
  connect(plot_, SIGNAL(zoomedOut()), this, SLOT(zoomedOut()));

  setLayout(fl);
}

void Consumer2D::update()
{
  if (!this->isVisible()
      || !plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  //  CustomTimer guiside(true);

  ConsumerMetadata md = consumer_->metadata();
  DataspacePtr data = consumer_->data();

  DataAxis axis_x, axis_y;
  EntryList spectrum_data;
  uint32_t res_x {0};
  uint32_t res_y {0};

//  bool buffered = md.get_attribute("buffered").triggered();
//  PreciseFloat total = md.get_attribute("total_count").get_number();

  if (data)
  {
    axis_x = data->axis(0);
    axis_y = data->axis(1);
    res_x = axis_x.bounds().second;
    res_y = axis_y.bounds().second;
    spectrum_data = data->range({{0, res_x}, {0, res_y}});
  }

//  if (!res_x || !res_y)
//  {
//    plot_->clearAll();
//    plot_->replot();
//    return;
//  }

  double rescale  = md.get_attribute("rescale").get_number();
  if (!std::isfinite(rescale) || !rescale)
    rescale = 1;

  if (!initial_scale_)
  {
    auto st = md.get_attribute("preferred_scale");
    auto scale = st.metadata().enum_name(st.selection());
    plot_->setScaleType(QString::fromStdString(scale));
    initial_scale_ = true;
  }

  QPlot::HistList2D hist;
  if (spectrum_data)
    for (auto p : *spectrum_data)
      hist.push_back(QPlot::p2d(p.first.at(0),
                                p.first.at(1),
                                rescale * to_double(p.second)));

  if (!hist.empty())
  {
    plot_->clearExtras();
    plot_->clearData();
    plot_->setAxes(
          QString::fromStdString(axis_x.label()), 0, res_x+1,
          QString::fromStdString(axis_y.label()), 0, res_y+1,
          "Event count");
    plot_->updatePlot(res_x + 1, res_y + 1, hist);
    plot_->replotExtras();
  }

  std::string new_label = md.get_attribute("name").get_text();
  setWindowTitle(QString::fromStdString(new_label).trimmed());

  if (!user_zoomed_)
    plot_->zoomOut();

  //  DBG << "<Plot2D> plotting took " << guiside.ms() << " ms";
}

void Consumer2D::refresh()
{
  plot_->replot();
}


void Consumer2D::mouseWheel (QWheelEvent *event)
{
  user_zoomed_ = true;
}

void Consumer2D::zoomedOut()
{
  user_zoomed_ = false;
}


