#pragma once

#include "AbstractConsumerWidget.h"
#include <QProgressBar>
#include <QLabel>

class ThermoMeter;
class ManoMeter;

class ConsumerScalar : public AbstractConsumerWidget
{
  Q_OBJECT

  public:
    ConsumerScalar(QWidget* parent = 0);

    void update() override;
    void refresh() override;

  private:
    ThermoMeter* thermometer_ {nullptr};
    ManoMeter* manometer_ {nullptr};
    QLabel* label_ {nullptr};
};
