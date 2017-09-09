#pragma once

#include "AbstractConsumerWidget.h"
#include "QPlot1D.h"

class Consumer1D : public AbstractConsumerWidget
{
  Q_OBJECT

public:
  Consumer1D(QWidget *parent = 0);

  void update() override;

private:
  QPlot::Multi1D* plot_ {nullptr};
  bool initial_scale_ {false};

};
