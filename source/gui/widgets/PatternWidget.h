#pragma once

#include <QObject>
#include <QMetaType>
#include <QWidget>
#include <QPointF>
#include <QVector>
#include <QStyledItemDelegate>
#include <vector>
#include <bitset>
#include <core/plugin/pattern.h>
#include <core/util/logger.h>

Q_DECLARE_METATYPE(DAQuiri::Pattern)

class PatternWidget : public QWidget
{
  Q_OBJECT
public:
  PatternWidget(QWidget *parent = 0);

  PatternWidget(DAQuiri::Pattern pattern = DAQuiri::Pattern(),
                double size = 25, size_t wrap = 0)
  {set_pattern(pattern, size, wrap);}

  void set_pattern(DAQuiri::Pattern pattern = DAQuiri::Pattern(),
                   double size = 25, size_t wrap = 0);

  QSize sizeHint() const Q_DECL_OVERRIDE;
  DAQuiri::Pattern pattern() const;

  void paint(QPainter *painter, const QRect &rect,
             const QPalette &palette) const;

protected:
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

private:

  int flagAtPosition(int x, int y);
  void setFlag(int count);

  QRectF outer, inner;
  std::vector<bool> pattern_;
  size_t threshold_;

  double size_;
  int wrap_, rows_;
};
