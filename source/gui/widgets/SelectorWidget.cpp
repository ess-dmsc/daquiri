#include <QtWidgets>
#include <math.h>

#include "SelectorWidget.h"
#include <core/util/logger.h>
#include <widgets/qt_util.h>

SelectorWidget::SelectorWidget(QWidget *parent)
  : QWidget(parent)
{
  setMouseTracking(true);
  setAutoFillBackground(true);
  max_wide = 3; //make this a parameter
  rect_w_ = 140;
  rect_h_ = 25;
  border = 5;
  selected_ = -1;
  height_total = 0;

  only_one_ = false;

  inner = QRectF(1, 1, rect_w_-1, rect_h_-1);
  outer = QRectF(2, 2, rect_w_-2, rect_h_-2);

  setToolTipDuration(10000); //hardcoded to 10 secs. Make this a parameter?
}

void SelectorWidget::set_only_one(bool o) {
  only_one_ = o;
}

void SelectorWidget::setItems(QVector<SelectorItem> items) {
  my_items_ = items;

  if (only_one_) {
    selected_ = -1;
    for (int i=0; i < my_items_.size(); ++i) {
      if (my_items_[i].visible) {
        selected_ = i;
        break;
      }
    }
    if ((selected_ == -1) && (!my_items_.empty()))
      selected_ = 0;
    for (int i=0; i < my_items_.size(); ++i)
      my_items_[i].visible = (i == selected_);
  } else
    selected_ = -1;

  height_total = my_items_.size() / max_wide;
  width_last = my_items_.size() % max_wide;
  if (width_last)
    height_total++;

  recalcDim(size().width(), size().height());
  update();
}

QVector<SelectorItem> SelectorWidget::items()
{
  return my_items_;
}

void SelectorWidget::recalcDim(int w, int h)
{
  Q_UNUSED(h)
  max_wide = w / rect_w_;
  height_total = my_items_.size() / max_wide;
  width_last = my_items_.size() % max_wide;
  if (width_last)
    height_total++;
  updateGeometry();
}

QSize SelectorWidget::sizeHint() const
{
  if (my_items_.empty())
//    return minimumSize();
    return QSize(rect_w_, rect_h_);
  else
    return QSize(max_wide*rect_w_, height_total*rect_h_);
}

QSize SelectorWidget::minimumSizeHint() const
{
  if (my_items_.empty()) {
    return QSize(rect_w_, rect_h_);
  } else {
    return QSize(max_wide*rect_w_, height_total*rect_h_);
  }
}


void SelectorWidget::resizeEvent(QResizeEvent * event) {
  recalcDim(event->size().width(), event->size().height());
}


void SelectorWidget::paintEvent(QPaintEvent *evt)
{
  QWidget::paintEvent(evt);

  QPainter painter(this);

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setPen(QPen(Qt::white));
  painter.setFont(QFont("Helvetica", 11, QFont::Normal));

  for (int i=0; i < my_items_.size(); i++)
  {
    int cur_high = i / max_wide;
    int cur_wide = i % max_wide;
    painter.resetTransform();
    painter.translate(rect().x() + cur_wide*rect_w_,
                      rect().y() + cur_high*rect_h_);

    QColor backg;
    QColor foreg;
    if (my_items_[i].visible)
    {
      backg = my_items_[i].color;
      foreg = Qt::white; //inverseColor(backg);
    }
    else
    {
      backg.setRgb(0,0,0,0);
      foreg = Qt::white;
    }

    if (backg.alpha() < 255)
    {
      QBrush b;
      b.setTexture(QPixmap(QStringLiteral(":/color_widgets/alphaback.png")));
      painter.fillRect(inner, b);
    }

    painter.setBrush(backg);
    painter.setPen(Qt::NoPen);
    painter.drawRect(inner);

    painter.setPen(QPen(foreg, 1));
    painter.drawText(inner, Qt::AlignLeft | Qt::AlignVCenter,
                     QString("  ") + my_items_[i].text);

    if (i == selected_)
    {
      painter.setBrush(Qt::NoBrush);
      painter.setPen(QPen(Qt::white, border));
      painter.drawRect(outer);
      painter.setPen(QPen(Qt::black, border/2));
      painter.drawRect(outer);
    }
  }
}


void SelectorWidget::mouseReleaseEvent(QMouseEvent *event)
{
  int flag = flagAt(event->x(), event->y());

  if (event->button()==Qt::LeftButton) {
    bool changed = (selected_ == flag);
    selected_ = flag;
    if (only_one_) {
      for (auto &q : my_items_)
        q.visible = false;
      if ((flag > -1) && (flag < my_items_.size()))
        my_items_[flag].visible = true;
    }
    update();
    if ((flag > -1) && (flag < my_items_.size()) && changed)
      emit itemSelected(my_items_[flag]);
    else
      emit itemSelected(SelectorItem());
  }

  if ((!only_one_) && (event->button()==Qt::RightButton) && (flag > -1) && (flag < my_items_.size())) {
    my_items_[flag].visible = !my_items_[flag].visible;
    update();
    emit itemToggled(my_items_[flag]);
  }
}


void SelectorWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
  int flag = flagAt(event->x(), event->y());

  if (event->button()==Qt::LeftButton) {
    selected_ = flag;
    update();
    if ((flag > -1) && (flag < my_items_.size()))
      emit itemDoubleclicked(my_items_[flag]);
    else
      emit itemDoubleclicked(SelectorItem());
  }

//  if ((event->button()==Qt::RightButton) && (flag > -1) && (flag < my_items_.size())) {
//    emit itemDoubleclicked(my_items_[flag]);
//  }
}


SelectorItem SelectorWidget::selected() {
  if ((selected_ > -1) && (selected_ < my_items_.size()))
    return my_items_[selected_];
  else
    return SelectorItem();
}

void SelectorWidget::replaceSelected(SelectorItem item) {
  if ((selected_ > -1) && (selected_ < my_items_.size()))
    my_items_[selected_] = item;
  update();
}

void SelectorWidget::setSelected(QString name) {
  for (int i=0; i < my_items_.size(); ++i)
    if (my_items_[i].text == name) {
      selected_ = i;
    }
  update();
}

int SelectorWidget::flagAt(int x, int y) {
  if (!my_items_.size())
    return -1;
  int phigh = y / rect_h_;
  int pwide = x / rect_w_;
  if ((phigh > height_total) || (pwide >= max_wide))
    return -1;
  if ((phigh == height_total) && (pwide > width_last))
    return -1;
  return (phigh * max_wide) + pwide;
}

void SelectorWidget::show_all() {
  for (auto &q : my_items_)
    if (!q.visible) {
      q.visible = true;
    }
  update();
}

void SelectorWidget::hide_all() {
  for (auto &q : my_items_)
    if (q.visible) {
      q.visible = false;
    }
  update();
}
