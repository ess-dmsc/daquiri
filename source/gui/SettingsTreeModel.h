#pragma once

#include <QAbstractTableModel>
#include <QAbstractItemModel>
#include <QFont>
#include <QBrush>
#include <core/plugin/setting.h>

using namespace DAQuiri;

class SettingsTreeItem
{
public:
    explicit SettingsTreeItem(const Setting &data, SettingsTreeItem *parent = 0);
    ~SettingsTreeItem();

    bool replace_data(const Setting &data);

    SettingsTreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant display_data(int column) const;
    QVariant edit_data(int column) const;
    bool is_editable(int column) const;
//    bool insertChildren(int position, int count, int columns);
    SettingsTreeItem *parent();
//    bool removeChildren(int position, int count);
    int childNumber() const;
    bool setData(int column, const QVariant &value);
    Setting rebuild();

private:
    QVector<SettingsTreeItem*> childItems;
    Setting itemData;
    SettingsTreeItem *parentItem;
};


class SettingsTreeModel : public QAbstractItemModel
{
  Q_OBJECT

private:
  Setting data_;
  SettingsTreeItem *getItem(const QModelIndex &index) const;
  SettingsTreeItem *rootItem;

  bool show_read_only_;
  bool show_address_;
  bool edit_read_only_;

public:
  explicit SettingsTreeModel(QObject *parent = 0);
  ~SettingsTreeModel();

  QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
  Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
  QModelIndex parent(const QModelIndex &index) const Q_DECL_OVERRIDE;
  int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
  int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
  bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) Q_DECL_OVERRIDE;

  void set_edit_read_only(bool edit_ro);
  void set_show_read_only(bool show_ro);
  void set_show_address(bool show_ad);

//  bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;
//  bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) Q_DECL_OVERRIDE;

  const Setting & get_tree();
  void update(const Setting &data);

signals:
  void tree_changed();
//  void detector_chosen(int chan, std::string name);

};
