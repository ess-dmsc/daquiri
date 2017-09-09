#pragma once

#include "AbstractConsumerWidget.h"
#include "QPlot2D.h"

class Consumer2D : public AbstractConsumerWidget
{
  Q_OBJECT

public:
  Consumer2D(QWidget *parent = 0);

  void update() override;

  void reset_content();

  void set_zoom(double);
  double zoom();

private slots:

  void crop_changed();


private:
  QPlot::Plot2D* plot_ {nullptr};

  double zoom_ {1.0};

  QMenu *crop_menu_;
  QLabel *crop_label_;
  QSlider *crop_slider_;

  bool initial_scale_ {false};
};
