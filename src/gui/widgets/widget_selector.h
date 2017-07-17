#pragma once

#include <QObject>
#include <QMetaType>
#include <QWidget>
#include <QPointF>
#include <QVector>
#include <QStaticText>
#include <QVariant>

struct SelectorItem {
  QString text;
  QColor color;
  bool visible;
  QVariant data;

  SelectorItem() : visible(false) {}
};

class SelectorWidget : public QWidget
{
  Q_OBJECT

public:
  SelectorWidget(QWidget *parent = 0);

  QSize sizeHint() const Q_DECL_OVERRIDE;
  QSize minimumSizeHint() const Q_DECL_OVERRIDE;
  void setItems(QVector<SelectorItem>);
  void replaceSelected(SelectorItem);
  void setSelected(QString);
  QVector<SelectorItem> items();
  SelectorItem selected();
  virtual void show_all();
  virtual void hide_all();
  void set_only_one(bool);

signals:
  void itemSelected(SelectorItem);
  void itemToggled(SelectorItem);
  void itemDoubleclicked(SelectorItem);

protected:
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent * event) Q_DECL_OVERRIDE;

  QVector<SelectorItem> my_items_;
  int selected_;

  int flagAt(int, int);

private:
  int rect_w_, rect_h_, border;
  int width_last, height_total, max_wide;

  QRectF inner, outer, text;
  bool   only_one_;

  void recalcDim(int, int);
};
