#pragma once

#include <QFileDialog>
#include <QTableWidget>
#include <QWidget>

QString QS(const std::string &s);

void add_to_table(QTableWidget *table, int row, int col, QString data,
                  QVariant value = QVariant(),
                  QBrush background = QBrush(Qt::white));

void clearLayout(QLayout *layout, bool deleteWidgets = true);
