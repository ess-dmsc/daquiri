#include <QtWidgets>
#include <math.h>
#include <limits>

#include <widgets/PatternWidget.h>
#include <core/util/logger.h>


void PatternWidget::set_pattern(DAQuiri::Pattern pattern, double size, size_t wrap)
{
  threshold_ = pattern.threshold();
  pattern_.clear();
  for (auto k : pattern.gates())
    if (k)
      pattern_.push_back(1);
    else
      pattern_.push_back(0);
  size_ = size;
  if (wrap > pattern_.size())
    wrap = pattern_.size();
  wrap_ = wrap;
  if (wrap > 0)
    rows_ = (pattern_.size() / wrap_) + ((pattern_.size() % wrap_) > 0);
  else
    rows_ = 0;

  outer = QRectF(2, 2, size_ - 4, size_ - 4);
  inner = QRectF(4, 4, size_ - 8, size_ - 8);
}

DAQuiri::Pattern PatternWidget::pattern() const
{
  DAQuiri::Pattern pt;
  pt.set_gates(pattern_);
  pt.set_threshold(threshold_);
  return pt;
}

int PatternWidget::flagAtPosition(int x, int y)
{
  if ((sizeHint().width() == 0) || (wrap_ <= 0))
    return -1;

  int flag_x = (x - 40) / ((sizeHint().width() - 40) / wrap_);
  int flag_y = y / (sizeHint().height() / rows_);
  int flag = flag_y * wrap_ + flag_x;

  if (flag < 0 || flag > static_cast<int>(pattern_.size()) || (x < 40) || (flag_x >= wrap_) || (flag_y >= rows_))
    return -1;

  return flag;
}

void PatternWidget::setFlag(int count)
{
  if ((count > -1) && (count < static_cast<int>(pattern_.size())))
  {
    pattern_[count] = !pattern_[count];
  }
}

QSize PatternWidget::sizeHint() const
{
  return QSize(wrap_ * size_ + 40, rows_ * size_);
}

void PatternWidget::paint(QPainter *painter, const QRect &rect,
                                 const QPalette &palette) const
{
  painter->save();

  size_t tally = 0;
  for (auto t : pattern_)
    if (t != 0)
      tally++;

  bool enabled = this->isEnabled();
  bool irrelevant = (threshold_ == 0);
  bool valid = (threshold_ <= tally);

  painter->setRenderHint(QPainter::Antialiasing, true);
  painter->setRenderHint(QPainter::TextAntialiasing, true);

  int flags = Qt::TextWordWrap | Qt::AlignVCenter;
  if (enabled)
    painter->setPen(palette.color(QPalette::Active, QPalette::Text));
  else
    painter->setPen(palette.color(QPalette::Disabled, QPalette::Text));
  painter->drawText(rect, flags, " " + QString::number(threshold_) + " of ");

  painter->setPen(Qt::NoPen);
  int yOffset = (rect.height() - (size_*rows_)) / 2;
  painter->translate(rect.x() + 40, rect.y() + yOffset);

  QColor on_color = enabled ? Qt::cyan : Qt::darkCyan;
  if (irrelevant)
    on_color = enabled ? Qt::green : Qt::darkGreen;
  if (!valid)
    on_color = enabled ? Qt::red : Qt::darkRed;
  QColor border = enabled ? Qt::black : Qt::darkGray;
  QColor text_color_rel = enabled ? Qt::black : Qt::lightGray;
  QColor text_color_irrel = enabled ? Qt::black : Qt::darkGray;

  for (int i = 0; i < rows_; ++i)
  {
    painter->translate(0, i * size_);
    for (int j = 0; j < wrap_; ++j)
    {
      int flag = i * wrap_ + j;

      if (flag < static_cast<int>(pattern_.size()))
      {

        painter->setPen(Qt::NoPen);
        painter->setBrush(border);
        painter->drawEllipse(outer);

        if (pattern_[flag])
          painter->setBrush(on_color);
        else
          painter->setBrush(palette.background());

        painter->drawEllipse(inner);

        if (pattern_[flag])
          painter->setPen(QPen(text_color_rel, 1));
        else
          painter->setPen(QPen(text_color_irrel, 1));

        painter->setFont(QFont("Helvetica", std::floor(8.0*(size_/25.0)), QFont::Bold));
        painter->drawText(outer, Qt::AlignCenter, QString::number(flag));
      }

      painter->translate(size_, 0.0);
    }
    painter->translate((-1)*size_*wrap_, 0.0);
  }

  painter->restore();
}


////Editor/////////////

PatternWidget::PatternWidget(QWidget *parent)
  : QWidget(parent)
{
  setMouseTracking(true);
  setAutoFillBackground(true);
  set_pattern();
}

void PatternWidget::paintEvent(QPaintEvent *)
{
  QPainter painter(this);
  paint(&painter, rect(), this->palette());
}


void PatternWidget::mouseReleaseEvent(QMouseEvent *event)
{
  int flag = flagAtPosition(event->x(), event->y());

  if (isEnabled() && (flag != -1))
  {
    setFlag(flag);
    update();
  }
}

void PatternWidget::wheelEvent(QWheelEvent *event)
{
  if (!event->angleDelta().isNull())
  {
    if (event->angleDelta().y() > 0)
      threshold_++;
    else if (threshold_ > 0)
      threshold_--;
    if (threshold_ > pattern_.size())
      threshold_ = pattern_.size();
    update();
  }
  event->accept();
}


