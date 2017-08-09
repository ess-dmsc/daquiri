#include "SettingsTableModel.h"
#include "SettingDelegate.h"
#include <boost/algorithm/string.hpp>
#include <QDateTime>
#include "qt_util.h"

SettingsTableModel::SettingsTableModel(QObject *parent)
  : QAbstractTableModel(parent)
{
  show_read_only_ = true;
  scalable_units_.insert("V");
  scalable_units_.insert("A");
  scalable_units_.insert("s");
  scalable_units_.insert("Hz");
}

void SettingsTableModel::set_show_read_only(bool show_ro) {
  show_read_only_ = show_ro;
}


int SettingsTableModel::rowCount(const QModelIndex & /*parent*/) const
{
//  if ((channels_.size() > 0) && (!consolidated_list_.branches.empty()))
    return consolidated_list_.branches.size() + 1;
//  else
//    return 0;
}

int SettingsTableModel::columnCount(const QModelIndex & /*parent*/) const
{
  int num = 3 + channels_.size();
  return num;
}

QVariant SettingsTableModel::data(const QModelIndex &index, int role) const
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::ForegroundRole)
  {
    if (row == 0) {
      if (col == 0) {
        QBrush brush(Qt::black);
        return brush;
      }
    } else if ((col <= static_cast<int>(channels_.size()+2)) && (!channels_.empty()) && (!consolidated_list_.branches.empty())) {
      Setting item = consolidated_list_.branches.get(row-1);
      if (!item.metadata().has_flag("readonly"))
      {
        QBrush brush(Qt::black);
        return brush;
      } else {
        QBrush brush(Qt::darkGray);
        return brush;
      }
    }
    else
      return QVariant();
  }

  else if (role == Qt::FontRole)
  {
    QFont font;
    font.setPointSize(10);
    if ((col == 0) && (row != 0))
      font.setBold(true);
    else if (col > static_cast<int>(channels_.size()))
      font.setItalic(true);
    return font;
  }


  else if (role == Qt::DisplayRole)
  {
    if (row == 0) {
      if (col == 0) {
        return "<===detector===>";
      }
      else if (col <= static_cast<int>(channels_.size()))
      {
        Setting det = Setting::detector("", channels_[col-1].id());
        det.add_indices({col-1});
        return QVariant::fromValue(det);
      } else
        return QVariant();
    } else {
      Setting item = consolidated_list_.branches.get(row-1);
      if (col == 0)
        return QString::fromStdString(item.id());
      else if ((col == static_cast<int>(channels_.size()+1)) && preferred_units_.count(item.id()))
        return QString::fromStdString(preferred_units_.at(item.id()));
      else if (col == static_cast<int>(channels_.size()+2))
        return QString::fromStdString(item.metadata().get_string("description",""));
      else if (col <= static_cast<int>(channels_.size()))
      {
        item = channels_[col-1].get_setting(item.id());
        if (item != Setting())
        {
          if (item.is(SettingType::floating))
          {
            double val = item.get_number();
            if (preferred_units_.count(item.id()) &&
                (item.metadata().get_string("units","") != preferred_units_.at(item.id())))
            {
              UnitConverter uc;
              val = to_double( uc.convert_units(val, item.metadata().get_string("units",""), preferred_units_.at(item.id())) );
            }
            return QVariant::fromValue(val);
          } else
            return QVariant::fromValue(item);
        }
      } else
        return QVariant();
    }
  }

  else if (role == Qt::EditRole) {
    if ((row == 0) && (col > 0) && (col <= static_cast<int>(channels_.size())))
    {
      Setting det = Setting::detector("", channels_[col-1].id());
      return QVariant::fromValue(det);
    }
    else if (row != 0)
    {
      Setting item = consolidated_list_.branches.get(row-1);
      if ((col == static_cast<int>(channels_.size()+1)) &&
          preferred_units_.count(item.id()))
      {
        UnitConverter uc;
        SettingMeta st("units", SettingType::menu);
        if (item.metadata().min<int64_t>())
        {
          for (auto a : uc.make_ordered_map(item.metadata().get_string("units",""),
                                            item.metadata().min<double>(),
                                            item.metadata().max<double>()))
            st.set_enum(a.first, a.second);
        }
        else
        {
          for (auto a : uc.make_ordered_map(item.metadata().get_string("units",""),
                                            item.metadata().step<double>(),
                                            item.metadata().max<double>()))
            st.set_enum(a.first, a.second);
        }

        Setting s(st);

        for (auto &q : st.enum_map())
          if (q.second == preferred_units_.at(item.id()))
            s.select(q.first);
        return QVariant::fromValue(s);
      }
      else if (col <= static_cast<int>(channels_.size()))
      {
        item = channels_[col-1].get_setting(item.id());
        auto md = item.metadata();
        if (item == Setting())
          return QVariant();
        if (item.is(SettingType::floating)
            && preferred_units_.count(item.id())
            && (item.metadata().get_string("units","") != preferred_units_.at(item.id())))
        {
          std::string from_units = md.get_string("units","");
          std::string to_units = preferred_units_.at(item.id());
          UnitConverter uc;
          md.set_val("min", to_double( uc.convert_units(md.min<double>(), from_units, to_units) ));
          md.set_val("step", to_double( uc.convert_units(md.step<double>(), from_units, to_units) ));
          md.set_val("max", to_double( uc.convert_units(md.max<double>(), from_units, to_units) ));
          item = Setting(md);
          item.set_number(to_double( uc.convert_units(item.get_number(), from_units, to_units) ));
        }
        return QVariant::fromValue(item);
      } else
        return QVariant();
    }
  }

  return QVariant();
}

QVariant SettingsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (orientation == Qt::Horizontal)
    {
      if (section == 0)
        return QString("Setting name");
      else if (section <= static_cast<int>(channels_.size()))
        return (QString("chan ") + QString::number(section-1));
      else if (section == static_cast<int>(channels_.size()+1))
        return "Units";
      else if (section == static_cast<int>(channels_.size()+2))
        return "Description";
    }
    else if (orientation == Qt::Vertical)
    {
      if (section)
        return QString::number(section-1);
      else
        return "D";
    }
  }
  else if (role == Qt::FontRole)
  {
    QFont boldfont;
    boldfont.setPointSize(10);
    boldfont.setCapitalization(QFont::AllUppercase);
    return boldfont;
  }
  return QVariant();
}

void SettingsTableModel::update(const std::vector<Detector> &settings)
{
  channels_ = settings;
  if (!show_read_only_)
  {
    for (size_t i=0; i < settings.size(); ++i)
    {
      channels_[i].clear_optimizations();
      channels_[i].add_optimizations(settings[i].optimizations(), true);
    }
  }

  consolidated_list_ = Setting();
  for (auto &q : channels_)
  {
    for (auto &p : q.optimizations())
    {
      consolidated_list_.branches.add(p);
      if (!preferred_units_.count(p.id()) && (!p.metadata().get_string("units","").empty()))
      {
        //        DBG << "adding preferred unit for " << p.id() << " as " << p.metadata().get_string("units","");
        preferred_units_[p.id()] = p.metadata().get_string("units","");
      }
    }
  }

  QModelIndex start_ix = createIndex( 0, 0 );
  QModelIndex end_ix = createIndex( rowCount() - 1, columnCount() - 1);
  emit dataChanged( start_ix, end_ix );
  emit layoutChanged();
}

Qt::ItemFlags SettingsTableModel::flags(const QModelIndex &index) const
{
  int row = index.row();
  int col = index.column();

  if ((col > 0) && (col <= static_cast<int>(channels_.size() + 2)) && (!consolidated_list_.branches.empty())) {
    if ((row == 0) && (col <= static_cast<int>(channels_.size())))
      return Qt::ItemIsEditable | QAbstractTableModel::flags(index);

    Setting item = consolidated_list_.branches.get(row-1);

    if (col == static_cast<int>(channels_.size() + 1)) {
      if (preferred_units_.count(item.id()) && scalable_units_.count(UnitConverter().strip_unit(item.metadata().get_string("units",""))))
        return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
      else
        return Qt::ItemIsEnabled | QAbstractTableModel::flags(index);
    } else if (col == static_cast<int>(channels_.size() + 2))
      return Qt::ItemIsEnabled | QAbstractTableModel::flags(index);

    item = channels_[col-1].get_setting(item.id());

    if (item == Setting())
      return QAbstractTableModel::flags(index);
    else if (!item.metadata().has_flag("readonly"))
      return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    else if (item.is(SettingType::binary))
      return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
    else
      return Qt::ItemIsEnabled | QAbstractTableModel::flags(index);
  }
  return QAbstractTableModel::flags(index);
}

bool SettingsTableModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  int row = index.row();
  int col = index.column();

  if (role == Qt::EditRole)
  {
    if ((row > 0) && (col <= static_cast<int>(channels_.size()+2)))
    {
      Setting item = consolidated_list_.branches.get(row-1);

      if (col == static_cast<int>(channels_.size()+1))
      {
        if (preferred_units_.count(item.id()))
        {
          int idx = value.toInt();
          std::string prefix;
          UnitConverter uc;
          if (idx < static_cast<int>(uc.prefix_values_indexed.size()))
            prefix = uc.prefix_values_indexed[idx];
          preferred_units_[item.id()] = prefix + uc.strip_unit(preferred_units_[item.id()]);
          QModelIndex start_ix = createIndex( row, 0 );
          QModelIndex end_ix = createIndex( row, columnCount() - 1);
          emit dataChanged( start_ix, end_ix );
          return true;
        } else
          return false;
      }

      item = channels_[col-1].get_setting(item.id());
      if (item == Setting())
        return false;

      if ((item.is(SettingType::menu)
           || item.is(SettingType::binary)
           || item.is(SettingType::command))
          && (value.canConvert(QMetaType::LongLong)))
        item.select(value.toLongLong());
      else if (item.is(SettingType::integer)
               && (value.type() == QVariant::LongLong))
        item.set_number(value.toLongLong());
      else if (item.is(SettingType::boolean)
               && (value.type() == QVariant::Bool))
        item.select(value.toBool());
      else if (item.is(SettingType::floating)
               && (value.type() == QVariant::Double))
      {
        double val = value.toDouble();
        if (preferred_units_.count(item.id()) &&
            (item.metadata().get_string("units","") != preferred_units_.at(item.id())))
        {
          UnitConverter uc;
          val = to_double( uc.convert_units(val, preferred_units_.at(item.id()), item.metadata().get_string("units","")) );
        }
        item.set_number(val);
      }
      else if (item.is(SettingType::precise)
               && (value.type() == QVariant::Double))
      {
        long double val = value.toDouble();
        if (preferred_units_.count(item.id()) &&
            (item.metadata().get_string("units","") != preferred_units_.at(item.id())))
        {
          UnitConverter uc;
          val = to_double( uc.convert_units(val, preferred_units_.at(item.id()), item.metadata().get_string("units","")) );
        }
        item.set_number(val);
      }
      else if ((item.is(SettingType::text)
                || item.is(SettingType::file)
                || item.is(SettingType::dir)
                || item.is(SettingType::detector) )
               && (value.type() == QVariant::String))
        item.set_text(value.toString().toStdString());
      else if (item.is(SettingType::time)
               && (value.type() == QVariant::DateTime))
        item.set_time(fromQDateTime(value.toDateTime()));
      else if (item.is(SettingType::duration)
               && (value.canConvert(QMetaType::LongLong)))
        item.set_duration(boost::posix_time::seconds(value.toLongLong()));

      emit setting_changed(item);
      return true;
    }
    else
    {
      emit detector_chosen(col-1, value.toString().toStdString());
      return true;
    }
  }
  return true;
}
