#include <QtWidgets>
#include <math.h>
#include <limits>

#include <widgets/KnightRiderWidget.h>
#include <core/util/custom_logger.h>

KnightRiderWidget::KnightRiderWidget(QWidget* parent)
    : QWidget(parent)
{
  QPalette pal = palette();
  pal.setColor(QPalette::Background, Qt::black);
  setPalette(pal);

  QFontDatabase fdb;

  int id1 = fdb.addApplicationFont(":/fonts/SF_Fedora_Titles.ttf");
  int id2 = fdb.addApplicationFont(":/fonts/Ruben.ttf");

//  if (id1 == -1)
//    INFO (":/fonts/SF_Fedora_Titles.ttf could not be loaded");
//  if (id2 == -1)
//    INFO (":/fonts/Ruben.ttf could not be loaded");
//
//  auto f = fdb.applicationFontFamilies(id1);
//  for (const auto& ff : f)
//    INFO ("Family: {}", ff.toStdString());
//
//  f = fdb.applicationFontFamilies(id2);
//  for (const auto& ff : f)
//    INFO ("Family2: {}", ff.toStdString());

//  setMouseTracking(true);
  setAutoFillBackground(true);
}

void KnightRiderWidget::paintEvent(QPaintEvent*)
{
  QPainter painter(this);
  paint(&painter, rect(), this->palette());
}

//QSize KnightRiderWidget::sizeHint() const
//{
//  return QSize(40, 12);
//}

void KnightRiderWidget::paint(QPainter* painter, const QRect& rect,
                              const QPalette& palette) const
{
  painter->save();

//  painter->setRenderHint(QPainter::Antialiasing, true);
//  painter->setRenderHint(QPainter::TextAntialiasing, true);


  int block_height_with_margin = block_height_ + 2 * block_margin_;
  int block_width = block_height_with_margin * 2;

  int height = rect.height() - 40;
  int count = height / block_height_with_margin;

  double block_weight = 0;
  if (count > 0)
    block_weight = max_ / static_cast<double>(count);

  double current_blocks = 0;
  if (block_weight > 0)
    current_blocks = val_ / block_weight;

  double fractional = current_blocks - std::floor(current_blocks);

//  INFO("Paint rect {}x{}", rect.width(), rect.height());
//  INFO("   bock count {}  remainder= {}", count, (height % 12));
  INFO("   val={}  on [{},{}]", val_, min_, max_);
  INFO("   bock weight={}  current_blocks={}", block_weight, current_blocks);

  int dark = 35;
  int range(255 - dark);

  QColor on_color = Qt::red;
  QColor off_color = on_color;
  off_color.setHsv(on_color.hsvHue(), dark, dark);

  QColor frac_color;
  frac_color.setHsv(
      on_color.hsvHue(),
      dark + range * fractional,
      dark + range * fractional);

//  INFO("   on_color={} {} {}",
//       on_color.hsvHue(), on_color.hsvSaturation(), on_color.value());
//  INFO("   off_color={} {} {}",
//       off_color.hsvHue(), off_color.hsvSaturation(), off_color.value());
  INFO("   fractional={}  frac_color={} {} {}",
       fractional, frac_color.hsvHue(), frac_color.hsvSaturation(), frac_color.value());

  QRect box(width() / 2 - (block_width / 2), block_margin_,
            block_width, block_height_);

  painter->translate(0, this->height() - 20);
  painter->setPen(Qt::white);
  painter->setFont(QFont("SF Fedora Titles", 16, 2));
  painter->drawText(rect, Qt::AlignTop | Qt::AlignHCenter,
                    QString::number(val_) + " " + suffix_);
  painter->translate(0, -20);

  painter->setPen(Qt::NoPen);
  for (int i = 1; i <= count; ++i)
  {
    painter->translate(0, -block_height_with_margin);

    QColor current_color = off_color;
    if (static_cast<double>(i) < current_blocks)
      current_color = on_color;
    if (static_cast<double>(i) == std::floor(current_blocks))
      current_color = frac_color;

//    INFO("      i={}  color={} {} {}", i,
//         current_color.hsvHue(),
//         current_color.hsvSaturation(),
//         current_color.value());

    //pen.setColor(current_color);
    //painter->setPen(pen);
    painter->setBrush(current_color);
    painter->drawRect(box);
  }
  painter->restore();
}

void KnightRiderWidget::setSuffix(const QString& s)
{
  suffix_ = s;
}

void KnightRiderWidget::setMinimum(double min)
{
  min_ = min;
}

void KnightRiderWidget::setMaximum(double max)
{
  max_ = max;
}

void KnightRiderWidget::setValue(double val)
{
  val_ = val;
}

double KnightRiderWidget::value() const
{
  return val_;
}

