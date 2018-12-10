#pragma once

#include "AbstractConsumerWidget.h"
#include <QPlot/QPlot2D.h>

class Consumer2D : public AbstractConsumerWidget
{
 Q_OBJECT

 public:
  Consumer2D(QWidget* parent = 0);

  void update() override;
  void refresh() override;

 private slots:
  void mouseWheel(QWheelEvent* event);
  void zoomedOut();
  void scaleChanged(QString);
  void gradientChanged(QString);
  void flipYChanged(bool);

  void clickedPlot(double x, double y, Qt::MouseButton button);

 private:
  QPlot::Plot2D* plot_{nullptr};
  bool initial_scale_{false};
  bool user_zoomed_{false};

  uint16_t box_x, box_y;
  bool box_visible {false};

  std::vector<double> x_domain;
  std::vector<double> y_domain;
};
