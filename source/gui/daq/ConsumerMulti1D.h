#pragma once

#include <gui/daq/AbstractConsumerWidget.h>
#include <QPlot/QPlot1D.h>

class ConsumerMulti1D : public AbstractConsumerWidget
{
 Q_OBJECT

 public:
  ConsumerMulti1D(QWidget* parent = 0);

  void update_data() override;
  void refresh() override;

 private slots:
  void mouseWheel(QWheelEvent* event);
  void zoomedOut();
  void scaleChanged(QString);

  void clickedPlot(double x, double y, Qt::MouseButton button);

 private:
  QPlot::Multi1D* plot_{nullptr};
  bool initial_scale_{false};
  bool user_zoomed_{false};

  QPlot::Marker1D marker;
};
