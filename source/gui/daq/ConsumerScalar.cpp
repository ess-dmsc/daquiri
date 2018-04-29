#include "ConsumerScalar.h"
#include <QVBoxLayout>

#include "manometer.h"
#include "thermometer.h"

#include "custom_logger.h"

using namespace DAQuiri;

ConsumerScalar::ConsumerScalar(QWidget *parent)
  : AbstractConsumerWidget(parent)
  , meter_ (new ManoMeter())
//  , meter_ (new ThermoMeter())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(meter_);

  meter_->setSizePolicy(QSizePolicy::MinimumExpanding,
                       QSizePolicy::MinimumExpanding);
  meter_->setSuffix(QString());
  meter_->setPrefix(QString());

  setLayout(fl);
}

void ConsumerScalar::update()
{
  if (!consumer_
      || (consumer_->dimensions() != 0))
    return;

  ConsumerMetadata md = consumer_->metadata();
  DataspacePtr data = consumer_->data();

  double rescale  = md.get_attribute("rescale").get_number();
  if (!std::isfinite(rescale) || !rescale)
    rescale = 1;

  DataAxis axis;
  EntryList spectrum_data;

  if (data)
  {
    axis = data->axis(0);
    meter_->setSuffix(QString::fromStdString(axis.label()));

    spectrum_data = data->range({});

    if (!data->empty() && spectrum_data && (spectrum_data->size() == 3))
    {
      auto it = spectrum_data->begin();
      double min = (it++)->second * rescale;
      double val = (it++)->second * rescale;
      double max = (it++)->second * rescale;

      meter_->setMinimum(min);
      meter_->setMaximum(max);
      meter_->setValue(val);
    }
  }

  std::string new_label = md.get_attribute("name").get_text();
  setWindowTitle(QString::fromStdString(new_label).trimmed());
}

void ConsumerScalar::refresh()
{
//  plot_->replot(QCustomPlot::rpQueuedRefresh);
}