#include "QColorExtensions.h"

QColor generateColor()
{
  int H = rand() % 359;
  int S = rand() % 64 + 191;
  int V = rand() % 54 + 181;
  int A = 128;
  return QColor::fromHsv(H, S, V, A);
}

QColor inverseColor(QColor c)
{
  return QColor::fromRgb(255 - c.red(),
                         255 - c.green(),
                         255 - c.blue());
}

void paintColor(QPainter* painter,
                const QRect &rect,
                const QColor& color,
                bool with_text)
{
  painter->save();

  if (color.alpha() < 255)
  {
    QBrush b;
    b.setTexture(QPixmap(QStringLiteral(":/color_widgets/alphaback.png")));
    painter->fillRect(rect, b);
  }

  painter->fillRect(rect, color);

  if (with_text)
  {
    painter->setPen(QPen(inverseColor(color), 4));
    QFont f = painter->font();
    f.setBold(true);
    painter->setFont(f);
    painter->drawText(rect, Qt::AlignCenter | Qt::AlignVCenter, color.name());
  }

  painter->restore();
}
