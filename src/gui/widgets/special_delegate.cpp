#include "special_delegate.h"
#include "setting.h"
#include "qt_util.h"
#include <QComboBox>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QFileInfo>
#include <QBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QColorDialog>
#include "time_duration_widget.h"

#include <QApplication>

void DAQuiriSpecialDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
  if (index.data().type() == QVariant::Color)
  {
    QColor thisColor = qvariant_cast<QColor>(index.data());
    painter->fillRect(option.rect, thisColor);
    return;
  }
  else if ((index.data().canConvert<DAQuiri::Setting>()) &&
      qvariant_cast<DAQuiri::Setting>(index.data()).is(DAQuiri::SettingType::color))
  {
    DAQuiri::Setting itemData = qvariant_cast<DAQuiri::Setting>(index.data());
    QColor thisColor(QString::fromStdString(itemData.get_text()));
    painter->fillRect(option.rect, thisColor);
    return;
  }
  else if ((option.state & QStyle::State_Selected) && !(option.state & QStyle::State_HasFocus))
  {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }


  if (index.data().canConvert<DAQuiri::Setting>())
  {
    DAQuiri::Setting itemData = qvariant_cast<DAQuiri::Setting>(index.data());
    if (itemData.is(DAQuiri::SettingType::command))
    {
      QStyleOptionButton button;
      button.rect = option.rect;
      button.text = QString::fromStdString(itemData.metadata().get_string("name", ""));
      if (!itemData.metadata().has_flag("readonly"))
        button.state = QStyle::State_Enabled;
      QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
    }
    else if (itemData.is(DAQuiri::SettingType::pattern))
    {
      DAQuiriPatternEditor pat(itemData.pattern(), 20, 8);
      pat.setEnabled(!itemData.metadata().has_flag("readonly"));
      if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());
      pat.paint(painter, option.rect, option.palette);
    }
    else if (itemData.is(DAQuiri::SettingType::color))
    {
      QColor thisColor(QString::fromStdString(itemData.get_text()));
      painter->fillRect(option.rect, thisColor);
    }
    else if (itemData.is(DAQuiri::SettingType::dir) ||
               itemData.is(DAQuiri::SettingType::text) ||
               itemData.is(DAQuiri::SettingType::integer) ||
               itemData.is(DAQuiri::SettingType::integer) ||
               itemData.is(DAQuiri::SettingType::binary) ||
               itemData.is(DAQuiri::SettingType::floating) ||
               itemData.is(DAQuiri::SettingType::detector) ||
               itemData.is(DAQuiri::SettingType::indicator) ||
               itemData.is(DAQuiri::SettingType::time) ||
               itemData.is(DAQuiri::SettingType::duration) ||
               itemData.is(DAQuiri::SettingType::precise) ||
               itemData.is(DAQuiri::SettingType::boolean) ||
               itemData.is(DAQuiri::SettingType::file))
    {

      int flags = Qt::TextWordWrap | Qt::AlignVCenter;

      std::string raw_txt = itemData.val_to_pretty_string();
      if (raw_txt.size() > 32)
        raw_txt = raw_txt.substr(0,32) + "...";
      raw_txt = " " + raw_txt + " ";
      QString text = QString::fromStdString(raw_txt);

      painter->save();

      if (itemData.is(DAQuiri::SettingType::indicator))
      {
        QColor bkgCol = QColor(QString::fromStdString(
              itemData.find(DAQuiri::Setting(itemData.metadata().enum_name(itemData.get_number())))
              .metadata().get_string("color", ""))
              );
        painter->fillRect(option.rect, bkgCol);
        painter->setPen(QPen(Qt::white, 3));
        QFont f = painter->font();
        f.setBold(true);
        painter->setFont(f);
        flags |= Qt::AlignCenter;
      }
      else if (itemData.is(DAQuiri::SettingType::detector))
      {
        QVector<QColor> palette {Qt::darkCyan, Qt::darkBlue, Qt::darkGreen, Qt::darkRed, Qt::darkYellow, Qt::darkMagenta, Qt::red, Qt::blue};
        uint16_t index = 0;
        if (itemData.indices().size())
          index = *itemData.indices().begin();
        painter->setPen(QPen(palette[index % palette.size()], 2));
        QFont f = painter->font();
        f.setBold(true);
        painter->setFont(f);
      }
      else
      {
        if (option.state & QStyle::State_Selected)
        {
          painter->fillRect(option.rect, option.palette.highlight());
          painter->setPen(option.palette.highlightedText().color());
        } else {
          if (itemData.metadata().has_flag("readonly"))
            painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));
          else
            painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
        }
      }

      painter->drawText(option.rect, flags, text);
      painter->restore();
    }
  }
  else
    QStyledItemDelegate::paint(painter, option, index);

}

QSize DAQuiriSpecialDelegate::sizeHint(const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
  if (index.data().canConvert<DAQuiri::Setting>())
  {
    DAQuiri::Setting itemData = qvariant_cast<DAQuiri::Setting>(index.data());
    if (itemData.is(DAQuiri::SettingType::command))
    {
      QPushButton button;
      button.setText(QString::fromStdString(itemData.metadata().get_string("name","")));
      return button.sizeHint();
    }
    else if (itemData.is(DAQuiri::SettingType::time))
    {
      QDateTimeEdit editor;
      editor.setCalendarPopup(true);
      editor.setTimeSpec(Qt::UTC);
      editor.setDisplayFormat("yyyy-MM-dd HH:mm:ss.zzz");
      QSize sz = editor.sizeHint();
      sz.setWidth(sz.width() + 20);
      return sz;
    }
    else if (itemData.is(DAQuiri::SettingType::pattern))
    {
      DAQuiriPatternEditor pattern(itemData.pattern(), 20, 8);
      return pattern.sizeHint();
    }
    else if (itemData.is(DAQuiri::SettingType::duration))
    {
      TimeDurationWidget editor;
      return editor.sizeHint();
    }
    else if (itemData.is(DAQuiri::SettingType::file) ||
             itemData.is(DAQuiri::SettingType::text) ||
             itemData.is(DAQuiri::SettingType::integer) ||
             itemData.is(DAQuiri::SettingType::menu) ||
             itemData.is(DAQuiri::SettingType::binary) ||
             itemData.is(DAQuiri::SettingType::floating) ||
             itemData.is(DAQuiri::SettingType::detector) ||
             itemData.is(DAQuiri::SettingType::color) ||
             itemData.is(DAQuiri::SettingType::command) ||
             itemData.is(DAQuiri::SettingType::time) ||
             itemData.is(DAQuiri::SettingType::duration) ||
             itemData.is(DAQuiri::SettingType::precise) ||
             itemData.is(DAQuiri::SettingType::indicator) ||
             itemData.is(DAQuiri::SettingType::boolean) ||
             itemData.is(DAQuiri::SettingType::dir))
    {
      std::string raw_txt = itemData.val_to_pretty_string();
      if (raw_txt.size() > 32)
        raw_txt = raw_txt.substr(0,32) + "...";
      raw_txt = " " + raw_txt + " ";
      QString text = QString::fromStdString(raw_txt);

      QRect r = option.rect;
      QFontMetrics fm(QApplication::font());
      QRect qr = fm.boundingRect(r, Qt::AlignLeft | Qt::AlignVCenter, text);
      QSize size(qr.size());
      return size;
    } else
      return QStyledItemDelegate::sizeHint(option, index);
  } else {
    return QStyledItemDelegate::sizeHint(option, index);
  }
}

QWidget *DAQuiriSpecialDelegate::createEditor(QWidget *parent,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index) const

{
  emit begin_editing();

  if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
  {
    DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
    if (set.is(DAQuiri::SettingType::floating)
        || set.is(DAQuiri::SettingType::precise))
    {
      QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
      return editor;
    }
    else if (set.is(DAQuiri::SettingType::integer))
    {
      QSpinBox *editor = new QSpinBox(parent);
      return editor;
    }
    else if (set.is(DAQuiri::SettingType::binary))
    {
      emit ask_binary(set, index);
      return nullptr;
    }
    else if (set.is(DAQuiri::SettingType::command) &&
               (!set.metadata().has_flag("readonly")))
    {
      emit ask_execute(set, index);
      return nullptr;
    }
    else if (set.is(DAQuiri::SettingType::text))
    {
      QLineEdit *editor = new QLineEdit(parent);
      return editor;
    }
    else if (set.is(DAQuiri::SettingType::color))
    {
//      QtColorPicker *editor = new QtColorPicker(parent, -1, true, 42);
//      connect(editor, SIGNAL(get_custom_color(QtColorPicker *)), this, SLOT(get_color_from_dialog(QtColorPicker *)));
//      return editor;
    }
    else if (set.is(DAQuiri::SettingType::time))
    {
      QDateTimeEdit *editor = new QDateTimeEdit(parent);
      editor->setCalendarPopup(true);
      editor->setTimeSpec(Qt::UTC);
      editor->setDisplayFormat("yyyy-MM-dd HH:mm:ss.zzz");
      return editor;
    }
    else if (set.is(DAQuiri::SettingType::duration))
    {
      TimeDurationWidget *editor = new TimeDurationWidget(parent);
      return editor;
    }
    else if (set.is(DAQuiri::SettingType::pattern))
    {
      DAQuiriPatternEditor *editor = new DAQuiriPatternEditor(parent);
      editor->set_pattern(set.pattern(), 20, 8);
      return editor;
    }
    else if (set.is(DAQuiri::SettingType::detector))
    {
      QComboBox *editor = new QComboBox(parent);
      editor->addItem(QString("none"), QString("none"));
//      for (size_t i=0; i < detectors_.size(); i++)
//      {
//        QString name = QString::fromStdString(detectors_.get(i).name());
//        editor->addItem(name, name);
//      }
      return editor;
    }
    else if (set.is(DAQuiri::SettingType::boolean))
    {
      QComboBox *editor = new QComboBox(parent);
      editor->addItem("True", QVariant::fromValue(true));
      editor->addItem("False", QVariant::fromValue(false));
      return editor;
    }
    else if (set.is(DAQuiri::SettingType::file))
    {
      QFileDialog *editor = new QFileDialog(parent, QString("Chose File"),
                                            QFileInfo(QString::fromStdString(set.get_text())).dir().absolutePath(),
                                            QString::fromStdString(set.metadata().get_string("wildcards","")));
      editor->setFileMode(QFileDialog::ExistingFile);

      return editor;
    }
    else if (set.is(DAQuiri::SettingType::dir))
    {
      QFileDialog *editor = new QFileDialog(parent, QString("Chose Directory"),
                                            QFileInfo(QString::fromStdString(set.get_text())).dir().absolutePath());
      editor->setFileMode(QFileDialog::Directory);

      return editor;
    }
    else if (set.is(DAQuiri::SettingType::menu))
    {
      QComboBox *editor = new QComboBox(parent);
      for (auto &q : set.metadata().enum_map())
        editor->addItem(QString::fromStdString(q.second), QVariant::fromValue(q.first));
      return editor;
    }
  }
  else if (index.data(Qt::EditRole).canConvert(QMetaType::QVariantList))
  {
    QLineEdit *editor = new QLineEdit(parent);
    return editor;
  }
  else if (index.data(Qt::EditRole).type() == QVariant::Double)
  {
    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
    editor->setDecimals(6);
    editor->setRange(std::numeric_limits<double>::min(),std::numeric_limits<double>::max());
    return editor;
  }
  else
  {
    return QStyledItemDelegate::createEditor(parent, option, index);
  }
}

void DAQuiriSpecialDelegate::setEditorData ( QWidget *editor, const QModelIndex &index ) const
{
  if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
  {
    if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
    {
      DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
      if (set.is(DAQuiri::SettingType::detector))
      {
        int cbIndex = cb->findText(QString::fromStdString(set.get_text()));
        if(cbIndex >= 0)
          cb->setCurrentIndex(cbIndex);
      }
      else if (set.is(DAQuiri::SettingType::boolean))
      {
        int cbIndex = cb->findData(QVariant::fromValue(set.get_number() != 0));
        if(cbIndex >= 0)
          cb->setCurrentIndex(cbIndex);
      }
      else if (set.metadata().enum_map().count(set.get_number()))
      {
        int cbIndex = cb->findText(QString::fromStdString(set.metadata().enum_name(set.get_number())));
        if(cbIndex >= 0)
          cb->setCurrentIndex(cbIndex);
      }
    }
  }
  else if (QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox *>(editor))
  {
    if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
    {
      DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
      sb->setRange(set.metadata().min<double>(), set.metadata().max<double>());
      sb->setSingleStep(set.metadata().step<double>());
      sb->setDecimals(6); //generalize
      sb->setValue(set.get_number());
    }
    else
      sb->setValue(index.data(Qt::EditRole).toDouble());
  }
  else if (QSpinBox *sb = qobject_cast<QSpinBox *>(editor))
  {
    if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
    {
      DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
      sb->setRange(static_cast<int64_t>(set.metadata().min<double>()), static_cast<int64_t>(set.metadata().max<double>()));
      sb->setSingleStep(static_cast<int64_t>(set.metadata().step<double>()));
      sb->setValue(static_cast<int64_t>(set.get_number()));
    } else
      sb->setValue(index.data(Qt::EditRole).toInt());
  }
  else if (QDateTimeEdit *dte = qobject_cast<QDateTimeEdit *>(editor))
  {
    if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
    {
      DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
      dte->setDateTime(fromBoostPtime(set.time()));
    }
  }
  else if (TimeDurationWidget *dte = qobject_cast<TimeDurationWidget *>(editor))
  {
    if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
    {
      DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
      dte->set_duration(set.duration());
    }
  }
//  else if (QtColorPicker *cp = qobject_cast<QtColorPicker *>(editor))
//  {
//    if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
//    {
//      DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
//      cp->setCurrentColor(QString::fromStdString(set.get_text()));
//      //      cp->setMoreColors(32);
//      //      cp->clickit();
//    }
//  }
  else if (QLineEdit *le = qobject_cast<QLineEdit *>(editor))
  {
    if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
    {
      DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
      le->setText(QString::fromStdString(set.get_text()));
    }
    else if (index.data(Qt::EditRole).canConvert(QMetaType::QVariantList))
    {
      QVariantList qlist = index.data(Qt::EditRole).toList();
      if (!qlist.isEmpty())
        qlist.pop_front(); //number of indices allowed;
      QString text;
      foreach (QVariant var, qlist)
      {
        text += QString::number(var.toInt()) + " ";
      }
      le->setText(text.trimmed());
    }
    else
      le->setText(index.data(Qt::EditRole).toString());
  }
  else if (QCheckBox *cb = qobject_cast<QCheckBox *>(editor))
    cb->setChecked(index.data(Qt::EditRole).toBool());
  else
    QStyledItemDelegate::setEditorData(editor, index);
}

void DAQuiriSpecialDelegate::setModelData ( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  if (DAQuiriPatternEditor *pe = qobject_cast<DAQuiriPatternEditor *>(editor))
  {
    model->setData(index, QVariant::fromValue(pe->pattern()), Qt::EditRole);
  }
  else if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
  {
    if (index.data(Qt::EditRole).canConvert<DAQuiri::Setting>())
    {
      DAQuiri::Setting set = qvariant_cast<DAQuiri::Setting>(index.data(Qt::EditRole));
      if (cb->currentData().type() == static_cast<QVariant::Type>(QMetaType::Bool))
        model->setData(index, QVariant::fromValue(cb->currentData().toBool()), Qt::EditRole);
      else if (cb->currentData().type() == static_cast<QVariant::Type>(QMetaType::Int))
        model->setData(index, QVariant::fromValue(cb->currentData().toInt()), Qt::EditRole);
      else if (cb->currentData().type() == static_cast<QVariant::Type>(QMetaType::Double))
        model->setData(index, QVariant::fromValue(cb->currentData().toDouble()), Qt::EditRole);
      else if (cb->currentData().type() == static_cast<QVariant::Type>(QMetaType::QString))
        model->setData(index, cb->currentData().toString(), Qt::EditRole);
    }
  }
  else if (QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox *>(editor))
    model->setData(index, QVariant::fromValue(sb->value()), Qt::EditRole);
  else if (QSpinBox *sb = qobject_cast<QSpinBox *>(editor))
    model->setData(index, QVariant::fromValue(sb->value()), Qt::EditRole);
  else if (QLineEdit *le = qobject_cast<QLineEdit *>(editor))
    model->setData(index, le->text(), Qt::EditRole);
  else if (QDateTimeEdit *dte = qobject_cast<QDateTimeEdit *>(editor))
    model->setData(index, dte->dateTime(), Qt::EditRole);
  else if (TimeDurationWidget *dte = qobject_cast<TimeDurationWidget *>(editor))
    model->setData(index, QVariant::fromValue(dte->get_duration()), Qt::EditRole);
//  else if (QtColorPicker *cp = qobject_cast<QtColorPicker *>(editor))
//    model->setData(index, cp->currentColor().name(QColor::HexArgb), Qt::EditRole);
  else if (QCheckBox *cb = qobject_cast<QCheckBox *>(editor))
    model->setData(index, QVariant::fromValue(cb->isChecked()), Qt::EditRole);
  else if (QFileDialog *fd = qobject_cast<QFileDialog *>(editor)) {
    if ((!fd->selectedFiles().isEmpty()) /*&& (validateFile(parent, fd->selectedFiles().front(), false))*/)
      model->setData(index, QVariant::fromValue(fd->selectedFiles().front()), Qt::EditRole);
  }
  else
    QStyledItemDelegate::setModelData(editor, model, index);
}

void DAQuiriSpecialDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}

//void DAQuiriSpecialDelegate::get_color_from_dialog(QtColorPicker *ed)
//{
//  this->blockSignals(true);
//  bool ok;
//  QRgb rgb = QColorDialog::getRgba(ed->currentColor().rgba(), &ok, ed);
//  if (ok) {
//    QColor col = QColor::fromRgba(rgb);
//    ed->setCurrentColor(col);
//  }
//  this->blockSignals(false);
//}
