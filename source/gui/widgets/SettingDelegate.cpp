#include <widgets/SettingDelegate.h>
#include <QComboBox>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QColorDialog>
#include "TimeDurationWidget.h"
#include "QtColorWidgets/color_selector.hpp"

#include <QApplication>

#include <QPlot/QPlot.h>

#include <widgets/QColorExtensions.h>
#include <widgets/QTimeExtensions.h>
#include <QPlot/GradientSelector.h>

using namespace color_widgets;
using namespace DAQuiri;

void SettingDelegate::set_detectors(const Container<Detector> &detectors)
{
  detectors_ = detectors;
}

void SettingDelegate::set_manifest(DAQuiri::StreamManifest manifest)
{
  stream_manifest_ = manifest;
}

void SettingDelegate::set_valid_streams(std::set<std::string> valid_streams)
{
  valid_streams_ = valid_streams;
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

QString SettingDelegate::get_string(const DAQuiri::Setting& val) const
{
  if (val.is(SettingType::indicator))
  {
    auto ii = val.find(Setting(val.metadata().enum_name(val.get_number())));
    QString text = QS(ii.metadata().get_string("name", ""));
    if (text.isEmpty())
      text = QS(ii.id());
    return text;
  }
  else if (val.is(SettingType::command))
  {
    QString txt = QS(val.metadata().get_string("command_name", ""));
    if (txt.isEmpty())
      txt = QS(val.id());
    return txt;
  }
  else if (val.is(SettingType::duration))
  {
    return QS(to_simple(val.duration()));
  }
  else if (val.is(SettingType::menu))
  {
    QString txt = QS(val.metadata().enum_name(val.get_number()));
    if (txt.isEmpty())
      txt = "["+ QString::number(val.get_number()) + "]";
    return txt;
  }

  // plain text

  QString text = QS(val.val_to_string());
  truncate_w_ellipses(text, text_len_limit_);
  return text;
}

void SettingDelegate::paint_detector(QPainter* painter, const QStyleOptionViewItem &option,
                                     const Setting &val) const
{
  painter->save();

  uint16_t idx = 0;
  if (val.indices().size())
    idx = *val.indices().begin();
  QString text = QS(val.get_text());
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
  QColor c(QS(val.get_text()));
  paintColor(painter, option.rect,
             (option.state & QStyle::State_Selected) ? inverseColor(c) : c,
             c.name());
}

void SettingDelegate::paint_gradient(QPainter* painter,
                                     const QStyleOptionViewItem &option,
                                     const Setting &val) const
{
  auto gname = QS(val.get_text());
  auto gs = QPlot::Gradients::defaultGradients();
  if (gs.contains(gname))
    QPlot::paintGradient(painter, option.rect, gs.get(gname));
  else
    paint_text(painter, option, val);
}

void SettingDelegate::paint_indicator(QPainter* painter, const QStyleOptionViewItem &option,
                                      const Setting &val) const
{
  auto ii = val.find(Setting(val.metadata().enum_name(val.get_number())));
  QColor color = QColor(QS(ii.metadata().get_string("color", "")));
  //  QString text = QS(ii.metadata().get_string("name", ""));

  paintColor(painter, option.rect,
             (option.state & QStyle::State_Selected) ? inverseColor(color) : color,
             get_string(val));
}

void SettingDelegate::paint_pattern(QPainter* painter, const QStyleOptionViewItem &option,
                                    const Setting &val) const
{
  PatternWidget pat(val.pattern(), pattern_vis_size_, pattern_chans_per_row_);
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
  button.text = get_string(val);
  if (!val.metadata().has_flag("readonly"))
    button.state = QStyle::State_Enabled;
  QApplication::style()->drawControl( QStyle::CE_PushButton, &button, painter);
}

void SettingDelegate::paint_text(QPainter* painter, const QStyleOptionViewItem &option,
                                 const Setting &val) const
{
  painter->save();
  text_flags(painter, option, val.metadata().has_flag("readonly"));
  painter->drawText(option.rect, Qt::AlignVCenter, get_string(val));
  painter->restore();
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
  else if (item.is(SettingType::text))
  {
    if (item.metadata().has_flag("color"))
      paint_color(painter, option, item);
    else if (item.metadata().has_flag("gradient-name"))
      paint_gradient(painter, option, item);
    else if (item.metadata().has_flag("detector"))
      paint_detector(painter, option, item);
    else
      paint_text(painter, option, item);
  }
  else
    paint_text(painter, option, item);
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
    button.setText(QS(item.metadata().get_string("name","")));
    return button.sizeHint();
  }
  else if (item.is(SettingType::pattern))
  {
    PatternWidget pattern(item.pattern(), pattern_vis_size_, pattern_chans_per_row_);
    return pattern.sizeHint();
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
  else if (item.is(SettingType::duration))
  {
    TimeDurationWidget editor;
    return editor.sizeHint();
  }
  else
  {
    QString text = get_string(item);

    QRect r = option.rect;
    QFontMetrics fm(QApplication::font());
    QRect qr = fm.boundingRect(r, Qt::AlignVCenter, text);
    QSize size(qr.size());
    return size;
  }

  return QStyledItemDelegate::sizeHint(option, index);
}

QWidget* SettingDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const

{
  emit begin_editing();

  if (!index.data(Qt::EditRole).canConvert<Setting>())
    return QStyledItemDelegate::createEditor(parent, option, index);

  Setting set = qvariant_cast<Setting>(index.data(Qt::EditRole));
  if (set.is(SettingType::floating) || set.is(SettingType::precise))
  {
    auto dsb = new QDoubleSpinBox(parent);
    dsb->setRange(set.metadata().min<double>(), set.metadata().max<double>());
    dsb->setSingleStep(set.metadata().step<double>());
    dsb->setDecimals(6); //generalize
    dsb->setValue(set.get_number());
    return dsb;
  }
  else if (set.is(SettingType::integer))
  {
    auto sb = new QSpinBox(parent);
    sb->setRange(set.metadata().min<int32_t>(),
                 set.metadata().max<int32_t>());
    sb->setSingleStep(set.metadata().step<int32_t>());
    sb->setValue(static_cast<int32_t>(set.get_number()));
    return sb;
  }
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
  else if (set.is(SettingType::pattern))
  {
    PatternWidget *editor = new PatternWidget(parent);
    editor->set_pattern(set.pattern(), 20, 8);
    return editor;
  }
  else if (set.is(SettingType::boolean))
  {
    auto cb = new QComboBox(parent);
    cb->addItem("True", QVariant::fromValue(true));
    cb->addItem("False", QVariant::fromValue(false));
    int cbIndex = cb->findData(QVariant::fromValue(set.get_number() != 0));
    if(cbIndex >= 0)
      cb->setCurrentIndex(cbIndex);
    return cb;
  }
  else if (set.is(SettingType::time))
  {
    auto te = new QDateTimeEdit(parent);
    te->setCalendarPopup(true);
    te->setTimeSpec(Qt::UTC);
    te->setDisplayFormat("yyyy-MM-dd HH:mm:ss.zzz");
    te->setDateTime(fromTimePoint(set.time()));
    return te;
  }
  else if (set.is(SettingType::duration))
  {
    auto td = new TimeDurationWidget(parent);
    td->set_duration(set.duration());
    return td;
  }
  else if (set.is(SettingType::menu))
  {
    auto cb = new QComboBox(parent);
    for (auto &q : set.metadata().enum_map())
      cb->addItem(QS(q.second),
                  QVariant::fromValue(q.first));
    //there is a better way to do this, indeces might be bad
    if (set.metadata().enum_map().count(set.get_number()))
    {
      int cbIndex = cb->findText(QS(set.metadata().enum_name(set.get_number())));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
    }
    return cb;
  }
  else if (set.is(SettingType::text))
  {
    if (set.metadata().has_flag("color"))
    {
      auto cs = new ColorSelector(parent);
      cs->setDisplayMode(ColorPreview::AllAlpha);
      cs->setUpdateMode(ColorSelector::Confirm);
      cs->setColor(QS(set.get_text()));
      return cs;
    }
    else if (set.metadata().has_flag("gradient-name"))
    {
      emit ask_gradient(QS(set.get_text()), index);
      return nullptr;
    }
    else if (set.metadata().has_flag("file"))
    {
      emit ask_file(set, index);
      return nullptr;
    }
    else if (set.metadata().has_flag("directory"))
    {
      emit ask_dir(set, index);
      return nullptr;
    }
    else if (set.metadata().has_flag("detector"))
    {
      auto cb = new QComboBox(parent);
      cb->addItem("none", "none");
      for (size_t i=0; i < detectors_.size(); i++)
      {
        QString name = QS(detectors_.get(i).id());
        cb->addItem(name, name);
      }
      int cbIndex = cb->findText(QS(set.get_text()));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
      return cb;
    }
    else if (set.metadata().has_flag("stream"))
    {
      auto cb = new QComboBox(parent);
      cb->addItem("<none>", "");
      for (auto stream : stream_manifest_)
      {
        QString name = QS(stream.first);
        cb->addItem(name, name);
      }
      int cbIndex = cb->findText(QS(set.get_text()));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
      return cb;
    }
    else if (set.metadata().has_flag("event_value"))
    {
      auto cb = new QComboBox(parent);
      cb->addItem("<none>", "");
      for (auto stream : stream_manifest_)
      {
        if (!valid_streams_.count(stream.first))
          continue;
        for (auto val : stream.second.event_model.value_names)
        {
          QString name = QS(val);
          cb->addItem(name, name);
        }
      }
      int cbIndex = cb->findText(QS(set.get_text()));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
      return cb;
    }
    else if (set.metadata().has_flag("event_trace"))
    {
      auto cb = new QComboBox(parent);
      cb->addItem("<none>", "");
      for (auto stream : stream_manifest_)
      {
        if (!valid_streams_.count(stream.first))
          continue;
        for (size_t i=0; i < stream.second.event_model.traces.size(); ++i)
        {
          QString name = QS(stream.second.event_model.trace_names[i]);
          QStringList ss;
          for (auto d : stream.second.event_model.traces[i])
            ss.push_back(QString::number(d));
          cb->addItem(name + " [" + ss.join(", ") + "]", name);
        }
      }
      int cbIndex = cb->findData(QS(set.get_text()));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
      return cb;
    }
    else if (set.metadata().has_flag("stat_value"))
    {
      auto cb = new QComboBox(parent);
      cb->addItem("<none>", "");
      for (auto stream : stream_manifest_)
      {
        if (!valid_streams_.count(stream.first))
          continue;
        for (const auto& set : stream.second.stats.branches)
        {
          QString name = QS(set.id());
          cb->addItem(name, name);
        }
      }
      int cbIndex = cb->findText(QS(set.get_text()));
      if(cbIndex >= 0)
        cb->setCurrentIndex(cbIndex);
      return cb;
    }
    else
    {
      auto le = new QLineEdit(parent);
      le->setText(QS(set.get_text()));
      return le;
    }
  }
  return QStyledItemDelegate::createEditor(parent, option, index);
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
