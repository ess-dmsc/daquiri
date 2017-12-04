#pragma once

#include <QStyledItemDelegate>
#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include "PatternWidget.h"
#include "detector.h"

Q_DECLARE_METATYPE(DAQuiri::Detector)
Q_DECLARE_METATYPE(DAQuiri::Setting)
Q_DECLARE_METATYPE(boost::posix_time::time_duration)


class SettingDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  SettingDelegate(QObject *parent = 0)
    : QStyledItemDelegate(parent) {}

  void paint(QPainter *painter,
             const QStyleOptionViewItem &option,
             const QModelIndex &index) const Q_DECL_OVERRIDE;

  QSize sizeHint(const QStyleOptionViewItem &option,
                 const QModelIndex &index) const Q_DECL_OVERRIDE;

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const Q_DECL_OVERRIDE;
  void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;

  void setModelData(QWidget *editor, QAbstractItemModel *model,
                    const QModelIndex &index) const Q_DECL_OVERRIDE;

  void updateEditorGeometry(QWidget *editor,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const Q_DECL_OVERRIDE;


  void set_detectors(const Container<DAQuiri::Detector>& dets);

  void text_len_limit(uint16_t tll);
  uint16_t text_len_limit() const;

signals:
  void begin_editing() const;
  void ask_execute(DAQuiri::Setting command, QModelIndex index) const;
  void ask_binary(DAQuiri::Setting command, QModelIndex index) const;

private:
  Container<DAQuiri::Detector> detectors_;

  void text_flags(QPainter* painter,
                  const QStyleOptionViewItem &option,
                  bool read_only) const;

  void paint_detector(QPainter* painter, const QStyleOptionViewItem &option,
                      const DAQuiri::Setting& val) const;

  void paint_color(QPainter* painter, const QStyleOptionViewItem &option,
                   const DAQuiri::Setting& val) const;

  void paint_indicator(QPainter* painter, const QStyleOptionViewItem &option,
                       const DAQuiri::Setting& val) const;

  void paint_pattern(QPainter* painter, const QStyleOptionViewItem &option,
                     const DAQuiri::Setting& val) const;

  void paint_command(QPainter* painter, const QStyleOptionViewItem &option,
                     const DAQuiri::Setting& val) const;

  void paint_duration(QPainter* painter, const QStyleOptionViewItem &option,
                      const DAQuiri::Setting& val) const;

  void paint_menu(QPainter* painter, const QStyleOptionViewItem &option,
                  const DAQuiri::Setting& val) const;

  void paint_text(QPainter* painter, const QStyleOptionViewItem &option,
                  const DAQuiri::Setting& val) const;

  void paint_generic(QPainter* painter, const QStyleOptionViewItem &option,
                     const QString& txt, bool read_only) const;

  void truncate_w_ellipses(QString& t, uint16_t max) const;

  uint16_t text_len_limit_ {80};

  QVector<QColor> detectors_palette_ {Qt::darkCyan,
        Qt::darkBlue, Qt::darkGreen,
        Qt::darkRed, Qt::darkYellow,
        Qt::darkMagenta, Qt::red, Qt::blue};
};
