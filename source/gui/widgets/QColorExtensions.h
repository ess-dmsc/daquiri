#pragma once

#include <QColor>
#include <QPainter>
#include <QRect>
#include <QGradient>

QColor generateColor();

QColor inverseColor(QColor);

void paintColor(QPainter* painter,
                const QRect& rect,
                const QColor& color,
                QString text = "");

void paintGradient(QPainter* painter,
                   const QRect& rect,
                   QLinearGradient colors,
                   QString text = "");
