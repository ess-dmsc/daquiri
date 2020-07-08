#pragma once

#include <QWidget>
#include <core/Consumer.h>
#include <QIcon>

class AbstractConsumerWidget : public QWidget
{
  Q_OBJECT

public:
  inline AbstractConsumerWidget(QWidget *parent = 0)
    : QWidget(parent)
  {
    setWindowIcon(QIcon(":/icons/noun_583391_cc_b.png"));
  }

  inline void setConsumer(DAQuiri::ConsumerPtr consumer)
  {
    consumer_ = consumer;
    update();
  }

  inline DAQuiri::ConsumerPtr consumer() const
  {
    return consumer_;
  }

  virtual void update_data() = 0;
  virtual void refresh() = 0;

protected:
  DAQuiri::ConsumerPtr consumer_;
};
