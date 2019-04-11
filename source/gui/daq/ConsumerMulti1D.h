#pragma once

#include <QPlot/QPlot1D.h>
#include <QWidget>
#include <core/consumer.h>
#include <QIcon>

class ConsumerMulti1D : public QWidget
{
 Q_OBJECT

 public:
  ConsumerMulti1D(QWidget* parent = 0);

  inline void setConsumer(DAQuiri::ConsumerPtr consumer)
  {
    consumer_ = consumer;
    update();
  }

  inline DAQuiri::ConsumerPtr consumer() const
  {
    return consumer_;
  }

  void update_data();
  void refresh();

 private slots:
  void mouseWheel(QWheelEvent* event);
  void zoomedOut();
  void scaleChanged(QString);

  void clickedPlot(double x, double y, Qt::MouseButton button);

 private:
  DAQuiri::ConsumerPtr consumer_;

  QPlot::Multi1D* plot_{nullptr};
  bool initial_scale_{false};
  bool user_zoomed_{false};

  QPlot::Marker1D marker;
};
