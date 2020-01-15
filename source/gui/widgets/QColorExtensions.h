#pragma once

#include <QColor>
#include <QPainter>
#include <QRect>

QColor generateColor();

QColor inverseColor(QColor);

void paintColor(QPainter *painter, const QRect &rect, const QColor &color,
                QString text = "");
