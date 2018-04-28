#include "ConsumerScalar.h"
#include <QVBoxLayout>

#include "custom_logger.h"
#include <QColor>

using namespace DAQuiri;

ConsumerScalar::ConsumerScalar(QWidget *parent)
  : AbstractConsumerWidget(parent)
  , plot_ (new QProgressBar())
  , label_ (new QLabel())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(plot_);
  fl->addWidget(label_);

//  plot_->setOpenGl(true);

  plot_->setSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::MinimumExpanding);
  connect(plot_, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel(QWheelEvent*)));
  connect(plot_, SIGNAL(zoomedOut()), this, SLOT(zoomedOut()));

  connect(plot_, SIGNAL(scaleChanged(QString)), this, SLOT(scaleChanged(QString)));

  setLayout(fl);
}

void ConsumerScalar::update()
{
  if (!plot_
      || !consumer_
      || (consumer_->dimensions() != 0))
    return;

  ConsumerMetadata md = consumer_->metadata();
  DataspacePtr data = consumer_->data();

  double rescale  = md.get_attribute("rescale").get_number();
  if (!std::isfinite(rescale) || !rescale)
    rescale = 1;

  if (!initial_scale_)
  {
    auto st = md.get_attribute("preferred_scale");
    auto scale = st.metadata().enum_name(st.selection());
//    plot_->setScaleType(QString::fromStdString(scale));
    initial_scale_ = true;
  }

  auto pen = QColor(QString::fromStdString(md.get_attribute("appearance").get_text()));

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

    if (spectrum_data)
    {
    }

    if (!data->empty() && (spectrum_data->size() == 3))
    {
      auto it = spectrum_data->begin();
      double min = (it++)->second;
      double val = (it++)->second;
      double max = (it++)->second;

      plot_->setMinimum(0);
      plot_->setMaximum(100);
      plot_->setValue((val - min) / (max - min) * 100);

      label_->setText(QString::number(val) +
      " on (" + QString::number(min) + "," + QString::number(max) + ")");
    }

  }


//  plot_->setAxisLabels(QString::fromStdString(axis.label()), "count");

  std::string new_label = md.get_attribute("name").get_text();
  setWindowTitle(QString::fromStdString(new_label).trimmed());

//  if (!user_zoomed_)
//    plot_->zoomOut();
}

void ConsumerScalar::scaleChanged(QString sn)
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

void ConsumerScalar::refresh()
{
//  plot_->replot(QCustomPlot::rpQueuedRefresh);
}

void ConsumerScalar::mouseWheel (QWheelEvent *event)
{
  Q_UNUSED(event)
  user_zoomed_ = true;
}

void ConsumerScalar::zoomedOut()
{
  user_zoomed_ = false;
}
