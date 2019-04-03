#include <widgets/qt_util.h>
#include <stdlib.h>
#include <QFile>
#include <QDateTime>
#include <QMessageBox>
#include <QLayout>
#include <QLayoutItem>
#include <core/util/logger.h>

QString QS(const std::string& s)
{
  return QString::fromStdString(s);
}

void add_to_table(QTableWidget *table, int row, int col,
                  QString data, QVariant value, QBrush background)
{
  QTableWidgetItem * item = new QTableWidgetItem(data);
  //  item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
  item->setFlags(item->flags() ^ Qt::ItemIsEditable);
  item->setData(Qt::UserRole, value);
  item->setData(Qt::BackgroundRole, background);
  table->setItem(row, col, item);
  //  DBG( "added " << data << " and " << value.toDouble();
}

//void add_to_table(QTableWidget *table,
//                  int row, int col, std::string data,
//                  QVariant value, QBrush background)
//{
//  add_to_table(table, row, col, QString::fromStdString(data),
//               value, background);
//}

void clearLayout(QLayout* layout, bool deleteWidgets)
{
  QLayoutItem* item;
  while (layout->count() && (item = layout->takeAt(0)))
  {
    QWidget* widget;
    if (deleteWidgets && (widget = item->widget()))
    {
      delete widget;
    }
    if (QLayout* childLayout = item->layout())
    {
      clearLayout(childLayout, deleteWidgets);
    }
    delete item;
  }
}
