#pragma once

#include <QStyledItemDelegate>
#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <widgets/PatternWidget.h>
#include <core/detector.h>
#include <core/spill.h>

Q_DECLARE_METATYPE(DAQuiri::Detector)
Q_DECLARE_METATYPE(DAQuiri::Setting)
Q_DECLARE_METATYPE(hr_duration_t)


class SettingDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  SettingDelegate(QObject *parent = 0)
    : QStyledItemDelegate(parent) {}

  void set_detectors(const Container<DAQuiri::Detector>& dets);
  void set_manifest(DAQuiri::StreamManifest);
  void set_valid_streams(std::set<std::string>);
  void text_len_limit(uint16_t tll);
  uint16_t text_len_limit() const;

  QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
  void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE;
  void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

signals:
  void begin_editing() const;
  void ask_execute(DAQuiri::Setting command, QModelIndex index) const;
  void ask_binary(DAQuiri::Setting command, QModelIndex index) const;
  void ask_gradient(QString gname, QModelIndex index) const;
  void ask_file(DAQuiri::Setting file_setting, QModelIndex index) const;
  void ask_dir(DAQuiri::Setting file_setting, QModelIndex index) const;

private:
  Container<DAQuiri::Detector> detectors_;
  DAQuiri::StreamManifest stream_manifest_;
  std::set<std::string> valid_streams_;

  void text_flags(QPainter* painter,
                  const QStyleOptionViewItem &option,
                  bool read_only) const;

  QString get_string(const DAQuiri::Setting& val) const;

  void paint_detector(QPainter* painter, const QStyleOptionViewItem &option, const DAQuiri::Setting& val) const;
  void paint_color(QPainter* painter, const QStyleOptionViewItem &option, const DAQuiri::Setting& val) const;
  void paint_gradient(QPainter* painter, const QStyleOptionViewItem &option, const DAQuiri::Setting& val) const;
  void paint_indicator(QPainter* painter, const QStyleOptionViewItem &option, const DAQuiri::Setting& val) const;
  void paint_pattern(QPainter* painter, const QStyleOptionViewItem &option, const DAQuiri::Setting& val) const;
  void paint_command(QPainter* painter, const QStyleOptionViewItem &option, const DAQuiri::Setting& val) const;
  void paint_text(QPainter* painter, const QStyleOptionViewItem &option, const DAQuiri::Setting& val) const;
  void truncate_w_ellipses(QString& t, uint16_t max) const;

  uint16_t text_len_limit_ {80};
  uint16_t pattern_vis_size_ {20};
  uint16_t pattern_chans_per_row_ {10};

  QVector<QColor> detectors_palette_ {Qt::darkCyan,
        Qt::darkBlue, Qt::darkGreen,
        Qt::darkRed, Qt::darkYellow,
        Qt::darkMagenta, Qt::red, Qt::blue};
};
