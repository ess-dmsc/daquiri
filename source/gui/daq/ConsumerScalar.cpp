#include "ConsumerScalar.h"
#include <QVBoxLayout>

#include <widgets/qt_util.h>

#include <QPlot/KnightRiderWidget.h>
#include <QPlot/GradientSelector.h>

#include <core/util/custom_logger.h>

using namespace DAQuiri;

ConsumerScalar::ConsumerScalar(QWidget* parent)
    : AbstractConsumerWidget(parent)
      , rider_(new QPlot::KnightRiderWidget())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(rider_);

  rider_->setMinimumWidth(40);
  rider_->setMinimumHeight(12);
  rider_->setSizePolicy(QSizePolicy::MinimumExpanding,
                        QSizePolicy::MinimumExpanding);
  rider_->setSuffix(QString());
  //rider_->setPrefix(QString());

  gradients_.addStandardGradients();
  gradients_.set("RedOnly", {"#ff0000", "#ff0000"});
  gradients_.set("GYR", {"#00ff00", "#ffff00", "#ff0000"});

  setLayout(fl);
//  layout()->setContentsMargins(0, 0, 0, 0);
//  layout()->setSpacing(0);
//  layout()->setMargin(0);
//  setContentsMargins(0, 0, 0, 0);
}

void ConsumerScalar::update()
{
  if (!consumer_
      || (consumer_->dimensions() != 0))
    return;

  ConsumerMetadata md = consumer_->metadata();
  DataspacePtr data = consumer_->data();

  auto app = md.get_attribute("appearance").get_text();

  rider_->gradient_ = gradients_.get("GYR");
  rider_->gradient_ = gradients_.get(QS(app));

  rider_->setVisible(true);
//    if (md.get_attribute("enforce_upper_limit").get_bool())
//      thermometer_->setCritical(md.get_attribute("upper_limit").get_number());

  double rescale = md.get_attribute("rescale").get_number();
  if (!std::isfinite(rescale) || !rescale)
    rescale = 1;

  DataAxis axis;

  if (data)
  {
    axis = data->axis(0);
    rider_->setSuffix(QS(axis.label()));

    EntryList range = data->range({});
    auto current = data->get({});

    if (!data->empty() && range && (range->size() == 2))
    {
      double min = range->begin()->second * rescale;
      double val = current * rescale;
      double max = range->rbegin()->second * rescale;

      rider_->setRange(min, max);
      rider_->setValue(val);
    }
  }

  std::string new_label = md.get_attribute("name").get_text();
  setWindowTitle(QS(new_label).trimmed());
}

void ConsumerScalar::refresh()
{
  rider_->repaint();
}