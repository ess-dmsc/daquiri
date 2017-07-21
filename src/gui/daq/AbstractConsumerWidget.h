#pragma once

#include <QWidget>
#include "consumer.h"

class AbstractConsumerWidget : public QWidget
{
  Q_OBJECT

public:
  inline AbstractConsumerWidget(QWidget *parent = 0)
    : QWidget(parent)
  {}

  inline void setConsumer(DAQuiri::ConsumerPtr consumer)
  {
    consumer_ = consumer;
    update();
  }

  inline DAQuiri::ConsumerPtr consumer() const
  {
    return consumer_;
  }

  virtual void update() = 0;

protected:
  DAQuiri::ConsumerPtr consumer_;
};
