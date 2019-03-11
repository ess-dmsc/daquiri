#pragma once

#include <QObject>
#include <QMetaType>
#include <QWidget>
#include <QPointF>
#include <QVector>
#include <QStyledItemDelegate>
#include <vector>
#include <bitset>

class KnightRiderWidget : public QWidget
{
  Q_OBJECT
public:
  KnightRiderWidget(QWidget *parent = 0);

//  QSize sizeHint() const Q_DECL_OVERRIDE;
  void paint(QPainter *painter, const QRect &rect,
             const QPalette &palette) const;

  void setSuffix(const QString&);
  void setMinimum(double);
  void setMaximum(double);
  void setValue(double);
  double value() const;

 protected:
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

  QString suffix_;
  double min_ {0};
  double max_ {100};
  double val_ {50};

  size_t block_height_ {10};
  size_t block_margin_ {2};
};
