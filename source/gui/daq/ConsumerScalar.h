#pragma once

#include "AbstractConsumerWidget.h"
#include <QProgressBar>
#include <QLabel>
#include "abstractmeter.h"

class ConsumerScalar : public AbstractConsumerWidget
{
    Q_OBJECT

  public:
    ConsumerScalar(QWidget *parent = 0);

    void update() override;
    void refresh() override;

  private:
//    ThermoMeter* meter_ {nullptr};
//    ManoMeter* meter_ {nullptr};
    AbstractMeter* meter_ {nullptr};
};
