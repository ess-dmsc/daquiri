#pragma once

#include <gui/daq/AbstractConsumerWidget.h>
#include <QPlot/GradientSelector.h>


namespace QPlot
{
class KnightRiderWidget;
}

class ConsumerScalar : public AbstractConsumerWidget
{
 Q_OBJECT

 public:
  ConsumerScalar(QWidget* parent = 0);

  void update_data() override;
  void refresh() override;

 private:
  QPlot::KnightRiderWidget* rider_{nullptr};
  QPlot::Gradients gradients_;

};
