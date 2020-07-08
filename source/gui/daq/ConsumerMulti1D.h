#pragma once

#include <QPlot/QPlot1D.h>
#include <core/Project.h>
#include <QWidget>

class ConsumerMulti1D : public QWidget
{
 Q_OBJECT

 static constexpr size_t invalid_group {0};

 public:
  ConsumerMulti1D(QWidget* parent = 0);

  inline void set_project(DAQuiri::ProjectPtr project, size_t group)
  {
    project_ = project;
    group_ = group;
    update();
  }

  inline size_t group() const
  {
    return group_;
  }

  void update_data();
  void refresh();

 private slots:
  void mouseWheel(QWheelEvent* event);
  void zoomedOut();
  void scaleChanged(QString);

  void clickedPlot(double x, double y, Qt::MouseButton button);

 private:
  DAQuiri::ProjectPtr project_;
  size_t group_ {invalid_group};

  QPlot::Multi1D* plot_{nullptr};
  bool initial_scale_{false};
  bool user_zoomed_{false};

  QPlot::Marker1D marker;
};
