#include "Consumer1D.h"
#include <QVBoxLayout>
#include "QHist.h"

#include <core/util/custom_logger.h>


using namespace DAQuiri;

Consumer1D::Consumer1D(QWidget *parent)
  : AbstractConsumerWidget(parent)
  , plot_ (new QPlot::Multi1D())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(plot_);

//  plot_->setOpenGl(true);

  plot_->setSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::MinimumExpanding);
  plot_->setLineThickness(2);
  connect(plot_, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel(QWheelEvent*)));
  connect(plot_, SIGNAL(zoomedOut()), this, SLOT(zoomedOut()));

  connect(plot_, SIGNAL(scaleChanged(QString)), this, SLOT(scaleChanged(QString)));

  setLayout(fl);
}

void Consumer1D::update()
{
  if (!plot_
      || !consumer_
      || (consumer_->dimensions() != 1))
    return;

  plot_->clearPrimary();

  ConsumerMetadata md = consumer_->metadata();
  DataspacePtr data = consumer_->data();

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

  auto pen = QPen(QColor(QString::fromStdString(md.get_attribute("appearance").get_text())), 1);

  DataAxis axis;
  EntryList spectrum_data;

  if (data)
  {
    axis = data->axis(0);
    auto bounds = axis.bounds();
    if (md.get_attribute("trim").get_bool() &&
        ((bounds.second - bounds.first) > 1))
    {
      bounds.second--;
    }

    spectrum_data = data->range({bounds});
  }

  QPlot::HistMap1D hist;
  if (spectrum_data)
  {
    for (auto it : *spectrum_data)
    {
      double xx = axis.domain[it.first[0]];
      double yy = to_double( it.second ) * rescale;
      hist[xx] = yy;
    }
  }

  if (!hist.empty())
  {
    plot_->addGraph(hist, pen);
    plot_->replotExtras();
  }

  plot_->setAxisLabels(QString::fromStdString(axis.label()), "count");

  std::string new_label = md.get_attribute("name").get_text();
  setWindowTitle(QString::fromStdString(new_label).trimmed());

  if (!user_zoomed_)
    plot_->zoomOut();
}

void Consumer1D::scaleChanged(QString sn)
{
  if (!plot_
      || !consumer_
      || (consumer_->dimensions() != 1))
    return;

  ConsumerMetadata md = consumer_->metadata();
  auto st = md.get_attribute("preferred_scale");
  for (auto e : st.metadata().enum_map())
  {
    if (e.second == sn.toStdString())
    {
      st.select(e.first);
      break;
    }
  }
  consumer_->set_attribute(st);
}

void Consumer1D::refresh()
{
  plot_->replot(QCustomPlot::rpQueuedRefresh);
}

void Consumer1D::mouseWheel (QWheelEvent *event)
{
  Q_UNUSED(event)
  user_zoomed_ = true;
}

void Consumer1D::zoomedOut()
{
  user_zoomed_ = false;
}
