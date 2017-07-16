#pragma once

#include <QWidget>
#include <QFileDialog>
#include <QTableWidget>
#include <boost/date_time.hpp>

//  Modified FileDialog by Dave Mateer
//  http://stackoverflow.com/users/183339/dave-mateer
QString CustomSaveFileDialog(QWidget *parent,
                           const QString &title,
                           const QString &directory,
                           const QString &filter);

bool validateFile(QWidget* parent, QString, bool);

QColor generateColor();

QDateTime fromBoostPtime(boost::posix_time::ptime);

boost::posix_time::ptime fromQDateTime(QDateTime);

QString catExtensions(std::list<std::string> exts);

QString catFileTypes(QStringList types);

void add_to_table(QTableWidget *table,
                  int row, int col, std::string data,
                  QVariant value = QVariant(), QBrush background = QBrush(Qt::white));

QString path_of_file(QString filename);

void clearLayout(QLayout* layout, bool deleteWidgets = true);
