#include "Consumer2D.h"
#include <widgets/qt_util.h>

#include <core/util/timer.h>

#include <core/util/custom_logger.h>

using namespace DAQuiri;

Consumer2D::Consumer2D(QWidget *parent)
  : AbstractConsumerWidget(parent)
  , plot_ (new QPlot::Plot2D())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(plot_);

//  plot_->setOpenGl(true);

  plot_->setSizePolicy(QSizePolicy::Preferred,
                       QSizePolicy::MinimumExpanding);
  plot_->setGradient("Spectrum2");
  plot_->setShowGradientLegend(true);
  plot_->setFlipY(true);
  plot_->clearAll();
  plot_->replotAll();
  connect(plot_, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel(QWheelEvent*)));
  connect(plot_, SIGNAL(zoomedOut()), this, SLOT(zoomedOut()));

  connect(plot_, SIGNAL(scaleChanged(QString)), this, SLOT(scaleChanged(QString)));
  connect(plot_, SIGNAL(gradientChanged(QString)), this, SLOT(gradientChanged(QString)));

  setLayout(fl);
}

void Consumer2D::update()
{
  if (!this->isVisible()
      || !plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  //  Timer guiside(true);

  DataspacePtr data = consumer_->data();
  ConsumerMetadata md = consumer_->metadata();

  std::string new_label = md.get_attribute("name").get_text();
  setWindowTitle(QS(new_label).trimmed());

  if (!initial_scale_)
  {
    auto st = md.get_attribute("preferred_scale");
    auto scale = st.metadata().enum_name(st.selection());
    plot_->setScaleType(QS(scale));

    auto app = md.get_attribute("appearance").get_text();
    plot_->setGradient(QS(app));
    initial_scale_ = true;
  }

  double rescale  = md.get_attribute("rescale").get_number();
  if (!std::isfinite(rescale) || !rescale)
    rescale = 1;

  QPlot::HistList2D hist;
  if (data)
  {
    auto spectrum_data = data->all_data();
    if (spectrum_data)
      for (const auto& p : *spectrum_data)
        hist.push_back(QPlot::p2d(p.first[0],
                                  p.first[1],
                                  rescale * to_double(p.second)));
  }

  if (!hist.empty())
  {
    DataAxis axis_x = data->axis(0);
    DataAxis axis_y = data->axis(1);

    uint32_t res_x = axis_x.bounds().second;
    uint32_t res_y = axis_y.bounds().second;

    plot_->clearExtras();
    plot_->clearData();
    plot_->setAxes(
          QS(axis_x.label()), axis_x.domain[0], axis_x.domain[res_x],
          QS(axis_y.label()), axis_y.domain[0], axis_y.domain[res_y],
          "count");
    plot_->updatePlot(res_x + 1, res_y + 1, hist);
    plot_->replotExtras();
  }

  if (!user_zoomed_)
    plot_->zoomOut();

  //  DBG( "<Plot2D> plotting took " << guiside.ms() << " ms";
}

void Consumer2D::scaleChanged(QString sn)
{
  if (!plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  ConsumerMetadata md = consumer_->metadata();
  auto st = md.get_attribute("preferred_scale");
  for (auto e : st.metadata().enum_map())
  {
    if (e.second == sn.toStdString())
    {
      DBG("Matched {}={}", e.second, e.first);
      st.select(e.first);
      break;
    }
  }
  consumer_->set_attribute(st);
}

void Consumer2D::gradientChanged(QString g)
{
  if (!plot_
      || !consumer_
      || (consumer_->dimensions() != 2))
    return;

  ConsumerMetadata md = consumer_->metadata();
  auto st = md.get_attribute("appearance");
  st.set_text(g.toStdString());
  consumer_->set_attribute(st);
}

void Consumer2D::refresh()
{
  plot_->replot(QCustomPlot::rpQueuedRefresh);
}


void Consumer2D::mouseWheel (QWheelEvent *event)
{
  Q_UNUSED(event)
  user_zoomed_ = true;
}

void Consumer2D::zoomedOut()
{
  user_zoomed_ = false;
}


