#pragma once

#include <QWidget>
#include <QFileDialog>
#include <QTableWidget>

//  Modified FileDialog by Dave Mateer
//  http://stackoverflow.com/users/183339/dave-mateer
QString CustomSaveFileDialog(QWidget *parent,
                           const QString &title,
                           const QString &directory,
                           const QString &filter);

bool validateFile(QWidget* parent, QString, bool);

QString catExtensions(std::list<std::string> exts);

QString catFileTypes(QStringList types);

QString path_of_file(QString filename);

bool copy_dir_recursive(QString from_dir, QString to_dir, bool replace_on_conflit);

