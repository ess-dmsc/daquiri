#include "QFileExtensions.h"
#include <stdlib.h>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>
#include <QLayout>
#include <QLayoutItem>
#include <core/util/logger.h>

QString CustomSaveFileDialog(QWidget *parent,
                             const QString &title,
                             const QString &directory,
                             const QString &filter)
{
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
  return QFileDialog::getSaveFileName(parent,
                                      title,
                                      directory,
                                      filter);
#else
  QFileDialog dialog(parent, title, directory, filter);
  if (parent)
    dialog.setWindowModality(Qt::WindowModal);

  QRegExp filter_regex(QLatin1String("(?:^\\*\\.(?!.*\\()|\\(\\*\\.)(\\w+)"));
  QStringList filters = filter.split(QLatin1String(";;"));
  if (!filters.isEmpty())
  {
    dialog.setNameFilter(filters.first());
    if (filter_regex.indexIn(filters.first()) != -1)
      dialog.setDefaultSuffix(filter_regex.cap(1));
  }
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  if (dialog.exec() == QDialog::Accepted)
  {
    QString file_name = dialog.selectedFiles().first();
    QFileInfo info(file_name);
    if (info.suffix().isEmpty() && !dialog.selectedNameFilter().isEmpty())
      if (filter_regex.indexIn(dialog.selectedNameFilter()) != -1)
      {
        QString extension = filter_regex.cap(1);
        file_name += QLatin1String(".") + extension;
      }

    //    QFile file(file_name);
    //    if (file.exists()) {
    //        QMessageBox msgBox;
    //        msgBox.setText("Replace?");
    //        msgBox.setInformativeText("File \'" + file_name + "\' already exists. Replace?");
    //        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    //        msgBox.setDefaultButton(QMessageBox::Cancel);
    //        if (msgBox.exec() != QMessageBox::Yes)
    //          return QString();
    //    }

    return file_name;
  }
  else
    return QString();

#endif  // Q_WS_MAC || Q_WS_WIN
}

bool validateFile(QWidget* parent, QString name, bool write)
{
  QFile file(name);
  if (name.isEmpty())
    return false;

  if (!write)
  {
    if (!file.exists())
    {
      QMessageBox::warning(parent, "Failed", "File does not exist.");
      return false;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
      QMessageBox::warning(parent, "Failed", "Could not open file for reading.");
      return false;
    }
  }
  else
  {
    if (file.exists() && !file.remove())
    {
      QMessageBox::warning(parent, "Failed", "Could not delete file.");
      return false;
    }
    if (!file.open(QIODevice::WriteOnly))
    {
      QMessageBox::warning(parent, "Failed", "Could not open file for writing.");
      return false;
    }
  }
  file.close();
  return true;
}

QString catExtensions(std::list<std::string> exts)
{
  QString ret;
  for (auto &p : exts)
  {
    ret += "*." + QString::fromStdString(p);
    if (p != exts.back())
      ret += " ";
  }
  return ret;
}

QString catFileTypes(QStringList types)
{
  QString ret;
  for (auto &q : types)
  {
    if (q != types.front())
      ret += ";;";
    ret += q;
  }
  return ret;
}

QString path_of_file(QString filename)
{
  QString ret;
  QFileInfo file(filename);
  if (file.absoluteDir().isReadable())
    ret = file.absoluteDir().absolutePath();
  return ret;
}

bool copy_dir_recursive(QString from_dir, QString to_dir, bool replace_on_conflit)
{
  QDir dir;
  dir.setPath(from_dir);

  from_dir += QDir::separator();
  to_dir += QDir::separator();

  foreach (QString copy_file, dir.entryList(QDir::Files))
  {
    QString from = from_dir + copy_file;
    QString to = to_dir + copy_file;

    if (QFile::exists(to))
    {
      if (replace_on_conflit)
      {
        if (QFile::remove(to) == false)
        {
          return false;
        }
      }
      else
      {
        continue;
      }
    }

    if (QFile::copy(from, to) == false)
    {
      return false;
    }
  }

  foreach (QString copy_dir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
  {
    QString from = from_dir + copy_dir;
    QString to = to_dir + copy_dir;

    if (dir.mkpath(to) == false)
    {
      return false;
    }

    if (copy_dir_recursive(from, to, replace_on_conflit) == false)
    {
      return false;
    }
  }

  return true;
}
