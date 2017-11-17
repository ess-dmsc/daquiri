#pragma once

#include <QAbstractTableModel>
#include <QFont>
#include <QBrush>
#include "detector.h"
#include "UnitConverter.h"

using namespace DAQuiri;

class SettingsTableModel : public QAbstractTableModel
{
    Q_OBJECT
private:
    std::vector<Detector> channels_;
    Setting consolidated_list_;
    std::map<std::string, std::string> preferred_units_;
    bool show_read_only_;

    std::set<std::string> scalable_units_;

public:
    SettingsTableModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    void update(const std::vector<Detector> &settings);
    void set_show_read_only(bool show_ro);

signals:
    void setting_changed(DAQuiri::Setting setting);
    void detector_chosen(int chan, std::string name);

};
