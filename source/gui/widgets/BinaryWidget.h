#pragma once

#include <QDialog>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include "setting.h"

class BinaryWidget : public QDialog
{
  Q_OBJECT

public:
  explicit BinaryWidget(DAQuiri::Setting setting, QWidget *parent = 0);
  DAQuiri::Setting get_setting() {return setting_;}

private slots:
  void change_setting();

private:
  DAQuiri::Setting      setting_;
  std::vector<QCheckBox*> boxes_;
  std::vector<QDoubleSpinBox*>  spins_;
  std::vector<QComboBox*> menus_;
};
