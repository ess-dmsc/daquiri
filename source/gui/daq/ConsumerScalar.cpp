#include "ConsumerScalar.h"
#include <QVBoxLayout>

#include <widgets/qt_util.h>

#include <widgets/AnalogWidgets/manometer.h>
#include <widgets/AnalogWidgets/thermometer.h>

#include <core/util/custom_logger.h>

using namespace DAQuiri;

ConsumerScalar::ConsumerScalar(QWidget* parent)
    : AbstractConsumerWidget(parent)
    , thermometer_(new ThermoMeter())
    , manometer_(new ManoMeter())
    , label_(new QLabel())
{
  QVBoxLayout* fl = new QVBoxLayout();
  fl->addWidget(thermometer_);
  fl->addWidget(manometer_);
  fl->addWidget(label_);

  thermometer_->setSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::MinimumExpanding);
  thermometer_->setSuffix(QString());
  thermometer_->setPrefix(QString());

  manometer_->setSizePolicy(QSizePolicy::MinimumExpanding,
                            QSizePolicy::MinimumExpanding);
  manometer_->setSuffix(QString());
  manometer_->setPrefix(QString());

  label_->setSizePolicy(QSizePolicy::MinimumExpanding,
                        QSizePolicy::MinimumExpanding);
  label_->setAlignment(Qt::AlignCenter);

  setLayout(fl);
}

void ConsumerScalar::update()
{
  if (!consumer_
      || (consumer_->dimensions() != 0))
    return;

  ConsumerMetadata md = consumer_->metadata();
  DataspacePtr data = consumer_->data();

  auto app = md.get_attribute("appearance").get_int();
  if (app == 1)
  {
    manometer_->setVisible(true);
    thermometer_->setVisible(false);
    label_->setVisible(false);

    if (md.get_attribute("enforce_upper_limit").get_bool())
      manometer_->setCritical(md.get_attribute("upper_limit").get_number());
  }
  else if (app == 2)
  {
    manometer_->setVisible(false);
    thermometer_->setVisible(true);
    label_->setVisible(false);

    if (md.get_attribute("enforce_upper_limit").get_bool())
      thermometer_->setCritical(md.get_attribute("upper_limit").get_number());
  }
  else
  {
    manometer_->setVisible(false);
    thermometer_->setVisible(false);
    label_->setVisible(true);


    QFont font = label_->font();
    QRect cRect = label_->contentsRect();

    if(!label_->text().isEmpty())
    {

      int fontSize = 1;

      while (true)
      {
        QFont f(font);
        f.setPixelSize(fontSize);
        QRect r = QFontMetrics(f).boundingRect(label_->text());
        if (r.height() <= cRect.height() && r.width() <= cRect.width())
          fontSize++;
        else
          break;
      }

      font.setPixelSize(fontSize);
      label_->setFont(font);
    }
  }

  double rescale = md.get_attribute("rescale").get_number();
  if (!std::isfinite(rescale) || !rescale)
    rescale = 1;

  DataAxis axis;

  if (data)
  {
    axis = data->axis(0);
    thermometer_->setSuffix(QS(axis.label()));
    manometer_->setSuffix(QS(axis.label()));

    EntryList range = data->range({});
    auto current = data->get({});

    if (!data->empty() && range && (range->size() == 2))
    {
      double min = range->begin()->second * rescale;
      double val = current;
      double max = range->rbegin()->second * rescale;

      thermometer_->setMinimum(min);
      thermometer_->setMaximum(max);
      thermometer_->setValue(val);

      manometer_->setMinimum(min);
      manometer_->setMaximum(max);
      manometer_->setValue(val);

      label_->setText(QString::number(val));
    }
  }

  std::string new_label = md.get_attribute("name").get_text();
  setWindowTitle(QS(new_label).trimmed());
}

void ConsumerScalar::refresh()
{
//  plot_->replot(QCustomPlot::rpQueuedRefresh);
}