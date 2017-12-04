#include "SettingDelegate.h"
#include "setting.h"
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
#include "TimeDurationWidget.h"
#include "color_selector.hpp"

#include <QApplication>

#include <QPlot.h>

#include "QColorExtensions.h"
#include "QTimeExtensions.h"

using namespace color_widgets;
using namespace DAQuiri;

void SettingDelegate::set_detectors(const Container<Detector> &detectors)
{
  detectors_ = detectors;
}

void SettingDelegate::text_len_limit(uint16_t tll)
{
  text_len_limit_ = tll;
}

uint16_t SettingDelegate::text_len_limit() const
{
  return text_len_limit_;
}

void SettingDelegate::truncate_w_ellipses(QString &t, uint16_t max) const
{
  if (t.length() > max)
    t.truncate(max);
  t = " " + t + " ";
}

void SettingDelegate::text_flags(QPainter* painter,
                                 const QStyleOptionViewItem &option,
                                 bool read_only) const
{
  if (option.state & QStyle::State_Selected)
  {
    painter->fillRect(option.rect, option.palette.highlight());
    painter->setPen(option.palette.highlightedText().color());
  }
  else
  {
    if (read_only)
      painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));
    else
      painter->setPen(option.palette.color(QPalette::Active, QPalette::Text));
  }
}

void SettingDelegate::paint_detector(QPainter* painter, const QStyleOptionViewItem &option,
                                     const Setting &val) const
{
  painter->save();

  uint16_t idx = 0;
  if (val.indices().size())
    idx = *val.indices().begin();
  QString text = QString::fromStdString(val.get_text());
  QColor col = detectors_palette_[idx % detectors_palette_.size()];

  if (option.state & QStyle::State_Selected)
  {
    painter->fillRect(option.rect, option.palette.highlight());
    col = inverseColor(col);
  }

  painter->setPen(QPen(col, 2));
  QFont f = painter->font();
  f.setBold(true);
  painter->setFont(f);

  painter->drawText(option.rect, Qt::AlignVCenter, text);
  painter->restore();
}

void SettingDelegate::paint_color(QPainter* painter,
                                  const QStyleOptionViewItem &option,
                                  const Setting &val) const
{
  QColor c(QString::fromStdString(val.get_text()));
  paintColor(painter, option.rect,
             (option.state & QStyle::State_Selected) ? inverseColor(c) : c,
             c.name());
}

void SettingDelegate::paint_indicator(QPainter* painter, const QStyleOptionViewItem &option,
                                      const Setting &val) const
{
  auto ii = val.find(Setting(val.metadata().enum_name(val.get_number())));
  QColor color = QColor(QString::fromStdString(ii.metadata().get_string("color", "")));
  QString text = QString::fromStdString(ii.metadata().get_string("name", ""));

  paintColor(painter, option.rect,
             (option.state & QStyle::State_Selected) ? inverseColor(color) : color,
             text);
}

void SettingDelegate::paint_pattern(QPainter* painter, const QStyleOptionViewItem &option,
                                    const Setting &val) const
{
  PatternWidget pat(val.pattern(), 20, 8);
  pat.setEnabled(!val.metadata().has_flag("readonly"));
  if (option.state & QStyle::State_Selected)
    painter->fillRect(option.rect, option.palette.highlight());
  pat.paint(painter, option.rect, option.palette);
}

void SettingDelegate::paint_command(QPainter* painter, const QStyleOptionViewItem &option,
                                    const Setting &val) const
{
  QStyleOptionButton button;
  button.rect = option.rect;
  button.text = QString::fromStdString(val.metadata().get_string("name", ""));
  if (!val.metadata().has_flag("readonly"))
    button.state = QStyle::State_Enabled;
  QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
}

void SettingDelegate::paint_generic(QPainter* painter, const QStyleOptionViewItem &option,
                                    const QString& txt, bool read_only) const
{
  painter->save();
  text_flags(painter, option, read_only);
  painter->drawText(option.rect, Qt::AlignVCenter, txt);
  painter->restore();
}


void SettingDelegate::paint_duration(QPainter* painter, const QStyleOptionViewItem &option,
                                     const Setting &val) const
{
  QString txt = QString::fromStdString(boost::posix_time::to_simple_string(val.duration()));
  paint_generic(painter, option, txt, val.metadata().has_flag("readonly"));
}

void SettingDelegate::paint_menu(QPainter* painter, const QStyleOptionViewItem &option,
                                 const Setting &val) const
{
  QString txt = QString::fromStdString(val.metadata().enum_name(val.get_number()));
  paint_generic(painter, option, txt, val.metadata().has_flag("readonly"));
}

void SettingDelegate::paint_text(QPainter* painter, const QStyleOptionViewItem &option,
                                 const Setting &val) const
{
  QString text = QString::fromStdString(val.val_to_string());
  truncate_w_ellipses(text, text_len_limit_);
  paint_generic(painter, option, text, val.metadata().has_flag("readonly"));
}

void SettingDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
  if (!index.data().canConvert<Setting>())
  {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }

  Setting item = qvariant_cast<Setting>(index.data());

  if (item.is(SettingType::command))
    paint_command(painter, option, item);
  else if (item.is(SettingType::pattern))
    paint_pattern(painter, option, item);
  else if (item.is(SettingType::indicator))
    paint_indicator(painter, option, item);
  else if (item.is(SettingType::duration))
    paint_duration(painter, option, item);
  else if (item.is(SettingType::menu))
    paint_menu(painter, option, item);
  else if (item.is(SettingType::text))
  {
    if (item.metadata().has_flag("color"))
      paint_color(painter, option, item);
    else if (item.metadata().has_flag("detector"))
      paint_detector(painter, option, item);
    else
      paint_text(painter, option, item);
  }
  else
  {
    QString text = QString::fromStdString(item.val_to_string());
    paint_generic(painter, option, text, item.metadata().has_flag("readonly"));
  }
}

QSize SettingDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
  if (!index.data().canConvert<Setting>())
    return QStyledItemDelegate::sizeHint(option, index);

  Setting item = qvariant_cast<Setting>(index.data());

  if (item.is(SettingType::command))
  {
    QPushButton button;
    button.setText(QString::fromStdString(item.metadata().get_string("name","")));
    return button.sizeHint();
  }
  else if (item.is(SettingType::time))
  {
    QDateTimeEdit editor;
    editor.setCalendarPopup(true);
    editor.setTimeSpec(Qt::UTC);
    editor.setDisplayFormat("yyyy-MM-dd HH:mm:ss.zzz");
    QSize sz = editor.sizeHint();
    sz.setWidth(sz.width() + 20);
    return sz;
  }
  else if (item.is(SettingType::pattern))
  {
    PatternWidget pattern(item.pattern(), 20, 16);
    return pattern.sizeHint();
  }
  else if (item.is(SettingType::duration))
  {
    TimeDurationWidget editor;
    return editor.sizeHint();
  }
  else
  {
    QString text = QString::fromStdString(item.val_to_string());
    if (item.is(SettingType::text))
      truncate_w_ellipses(text, text_len_limit_);

    QRect r = option.rect;
    QFontMetrics fm(QApplication::font());
    QRect qr = fm.boundingRect(r, Qt::AlignVCenter, text);
    QSize size(qr.size());
    return size;
  }

  return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *SettingDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const

{
  emit begin_editing();

  if (!index.data(Qt::EditRole).canConvert<Setting>())
    return QStyledItemDelegate::createEditor(parent, option, index);

  Setting set = qvariant_cast<Setting>(index.data(Qt::EditRole));
  if (set.is(SettingType::floating) || set.is(SettingType::precise))
    return new QDoubleSpinBox(parent);
  else if (set.is(SettingType::integer))
    return new QSpinBox(parent);
  else if (set.is(SettingType::duration))
    return new TimeDurationWidget(parent);
  else if (set.is(SettingType::binary))
  {
    emit ask_binary(set, index);
    return nullptr;
  }
  else if (set.is(SettingType::command) &&
           !set.metadata().has_flag("readonly"))
  {
    emit ask_execute(set, index);
    return nullptr;
  }
  else if (set.is(SettingType::time))
  {
    QDateTimeEdit *editor = new QDateTimeEdit(parent);
    editor->setCalendarPopup(true);
    editor->setTimeSpec(Qt::UTC);
    editor->setDisplayFormat("yyyy-MM-dd HH:mm:ss.zzz");
    return editor;
  }
  else if (set.is(SettingType::pattern))
  {
    PatternWidget *editor = new PatternWidget(parent);
    editor->set_pattern(set.pattern(), 20, 8);
    return editor;
  }
  else if (set.is(SettingType::boolean))
  {
    QComboBox *editor = new QComboBox(parent);
    editor->addItem("True", QVariant::fromValue(true));
    editor->addItem("False", QVariant::fromValue(false));
    return editor;
  }
  else if (set.is(SettingType::text))
  {
    if (set.metadata().has_flag("color"))
    {
      ColorSelector *editor = new ColorSelector(parent);
      editor->setDisplayMode(ColorPreview::AllAlpha);
      editor->setUpdateMode(ColorSelector::Confirm);
      return editor;
    }
    else if (set.metadata().has_flag("gradient-name"))
    {
      QComboBox *editor = new QComboBox(parent);
      for (auto &q : QPlot::Gradients::defaultGradients().names())
        editor->addItem(q, q);
      return editor;
    }
    else if (set.metadata().has_flag("file"))
    {
      QFileDialog *editor = new QFileDialog(parent, QString("Chose File"),
                                            QFileInfo(QString::fromStdString(set.get_text())).dir().absolutePath(),
                                            QString::fromStdString(set.metadata().get_string("wildcards","")));
      editor->setFileMode(QFileDialog::ExistingFile);
      return editor;
    }
    else if (set.metadata().has_flag("directory"))
    {
      QFileDialog *editor = new QFileDialog(parent, QString("Chose Directory"),
                                            QFileInfo(QString::fromStdString(set.get_text())).dir().absolutePath());
      editor->setFileMode(QFileDialog::Directory);
      return editor;
    }
    else if (set.metadata().has_flag("detector"))
    {
      QComboBox *editor = new QComboBox(parent);
      editor->addItem("none", "none");
      for (size_t i=0; i < detectors_.size(); i++)
      {
        QString name = QString::fromStdString(detectors_.get(i).id());
        editor->addItem(name, name);
      }
      return editor;
    }
    else
      return new QLineEdit(parent);
  }
  else if (set.is(SettingType::menu))
  {
    QComboBox *editor = new QComboBox(parent);
    for (auto &q : set.metadata().enum_map())
      editor->addItem(QString::fromStdString(q.second),
                      QVariant::fromValue(q.first));
    return editor;
  }
  return QStyledItemDelegate::createEditor(parent, option, index);
}

void SettingDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
  if (!index.data(Qt::EditRole).canConvert<Setting>())
  {
    QStyledItemDelegate::setEditorData(editor, index);
    return;
  }
  Setting set = qvariant_cast<Setting>(index.data(Qt::EditRole));

  if (QComboBox *cb = qobject_cast<QComboBox*>(editor))
  {
    if (set.is(SettingType::boolean))
    {
      int cbIndex = cb->findData(QVariant::fromValue(set.get_number() != 0));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
    }
    else if (set.is(SettingType::menu) &&
             set.metadata().enum_map().count(set.get_number()))
    {
      int cbIndex = cb->findText(QString::fromStdString(set.metadata().enum_name(set.get_number())));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
    }
    else
    {
      int cbIndex = cb->findText(QString::fromStdString(set.get_text()));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
    }

  }
  else if (QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox*>(editor))
  {
    sb->setRange(set.metadata().min<double>(), set.metadata().max<double>());
    sb->setSingleStep(set.metadata().step<double>());
    sb->setDecimals(6); //generalize
    sb->setValue(set.get_number());
  }
  else if (QSpinBox *sb = qobject_cast<QSpinBox*>(editor))
  {
    sb->setRange(set.metadata().min<int32_t>(),
                 set.metadata().max<int32_t>());
    sb->setSingleStep(set.metadata().step<int32_t>());
    sb->setValue(static_cast<int32_t>(set.get_number()));
  }
  else if (QDateTimeEdit *dte = qobject_cast<QDateTimeEdit*>(editor))
    dte->setDateTime(fromBoostPtime(set.time()));
  else if (TimeDurationWidget *dte = qobject_cast<TimeDurationWidget*>(editor))
    dte->set_duration(set.duration());
  else if (ColorSelector *cp = qobject_cast<ColorSelector*>(editor))
    cp->setColor(QString::fromStdString(set.get_text()));
  else if (QLineEdit *le = qobject_cast<QLineEdit*>(editor))
    le->setText(QString::fromStdString(set.get_text()));
  else
    QStyledItemDelegate::setEditorData(editor, index);
}

void SettingDelegate::setModelData(QWidget *editor,
                                   QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  if (PatternWidget *pe = qobject_cast<PatternWidget*>(editor))
    model->setData(index, QVariant::fromValue(pe->pattern()), Qt::EditRole);
  else if (QComboBox *cb = qobject_cast<QComboBox*>(editor))
    model->setData(index, cb->currentData(), Qt::EditRole);
  else if (QDoubleSpinBox *sb = qobject_cast<QDoubleSpinBox*>(editor))
    model->setData(index, QVariant::fromValue(sb->value()), Qt::EditRole);
  else if (QSpinBox *sb = qobject_cast<QSpinBox*>(editor))
    model->setData(index, QVariant::fromValue(sb->value()), Qt::EditRole);
  else if (QLineEdit *le = qobject_cast<QLineEdit*>(editor))
    model->setData(index, le->text(), Qt::EditRole);
  else if (QDateTimeEdit *dte = qobject_cast<QDateTimeEdit*>(editor))
    model->setData(index, dte->dateTime(), Qt::EditRole);
  else if (TimeDurationWidget *dte = qobject_cast<TimeDurationWidget*>(editor))
    model->setData(index, QVariant::fromValue(dte->get_duration()), Qt::EditRole);
  else if (ColorSelector *cp = qobject_cast<ColorSelector*>(editor))
    model->setData(index, cp->color().name(QColor::HexArgb), Qt::EditRole);
  else if (QCheckBox *cb = qobject_cast<QCheckBox*>(editor))
    model->setData(index, QVariant::fromValue(cb->isChecked()), Qt::EditRole);
  else if (QFileDialog *fd = qobject_cast<QFileDialog*>(editor)) {
    if ((!fd->selectedFiles().isEmpty()) /*&& (validateFile(parent, fd->selectedFiles().front(), false))*/)
      model->setData(index, QVariant::fromValue(fd->selectedFiles().front()), Qt::EditRole);
  }
  else
    QStyledItemDelegate::setModelData(editor, model, index);
}

void SettingDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
  Q_UNUSED(index);
  editor->setGeometry(option.rect);
}
