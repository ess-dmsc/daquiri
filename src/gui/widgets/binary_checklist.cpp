#include "binary_checklist.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <bitset>

using namespace DAQuiri;

BinaryChecklist::BinaryChecklist(Setting setting, QWidget *parent) :
  QDialog(parent),
  setting_(setting)
{
  int wordsize = setting.metadata().get_num("bits", int(0));
  if (wordsize < 0)
    wordsize = 0;
  if (wordsize > 64)
    wordsize = 64;

  std::bitset<64> bs(setting.selection());

  QLabel *label;
  QFrame* line;

  QVBoxLayout *vl_bit    = new QVBoxLayout();
  label = new QLabel();
  label->setFixedHeight(25);
  label->setText("Bit");
  vl_bit->addWidget(label);

  QVBoxLayout *vl_checks = new QVBoxLayout();
  label = new QLabel();
  label->setFixedHeight(25);
  label->setText("Value");
  vl_checks->addWidget(label);

  QVBoxLayout *vl_descr  = new QVBoxLayout();
  label = new QLabel();
  label->setFixedHeight(25);
  label->setText("Description");
  vl_descr->addWidget(label);

  bool showall = false;
  for (int i=0; i < wordsize; ++i)
  {
    bool exists = setting_.metadata().enum_map().count(i);
    bool visible = setting_.metadata().enum_map().count(i) || showall;

    bool is_branch = false;
    Setting sub;
    if (exists)
      for (auto &q : setting_.branches)
        if (q.id() == setting_.metadata().enum_name(i))
        {
          is_branch = true;
          sub = q;
        }

    bool is_int = (is_branch && sub.is(SettingType::integer));
    bool is_menu = (is_branch && sub.is(SettingType::menu));

    label = new QLabel();
    label->setFixedHeight(25);
    label->setText(QString::number(i));
    label->setVisible(visible);
    vl_bit->addWidget(label);

    QCheckBox *box = new QCheckBox();
    box->setLayoutDirection(Qt::RightToLeft);
    box->setFixedHeight(25);
    box->setChecked(bs[i] && !is_branch);
    box->setEnabled(!setting.metadata().has_flag("readonly")
                    && setting_.metadata().enum_map().count(i) && !is_branch);
    box->setVisible(visible && !is_branch);
    boxes_.push_back(box);
    if (!is_branch)
      vl_checks->addWidget(box);

    QDoubleSpinBox *spin = new QDoubleSpinBox();
    spin->setFixedHeight(25);
    spin->setEnabled(!setting.metadata().has_flag("readonly")
                     && setting_.metadata().enum_map().count(i) && is_int);
    spin->setVisible(visible && is_int);
    spins_.push_back(spin);
    if (is_int)
    {
      uint64_t num = setting.selection();
      int bits = sub.metadata().get_num("bits", int(0));
      num = num << uint8_t(64 - bits - i);
      num = num >> uint8_t(64 - bits);
      uint64_t max = pow(2, uint16_t(bits)) - 1;

      double val_offset = sub.metadata().get_num("offset", double(0));
      double val_step = sub.metadata().get_num("step", double(1));

      spin->setMinimum(val_offset);
      spin->setSingleStep(val_step);
      spin->setMaximum(val_offset + val_step * max);
      spin->setValue(val_offset + val_step * num);
      if (val_step == 1.0)
        spin->setDecimals(0);
      vl_checks->addWidget(spin);
    }

    QComboBox *menu = new QComboBox();
    menu->setFixedHeight(25);
    menu->setEnabled(!setting.metadata().has_flag("readonly")
                     && setting_.metadata().enum_map().count(i) && is_menu);
    menu->setVisible(visible && is_menu);
    menus_.push_back(menu);
    if (is_menu)
    {
      int max = 0;
      for (auto q : sub.metadata().enum_map())
        if (q.first > max)
          max = q.first;
      int bits = log2(max);
      if (pow(2, bits) < max)
        bits++;

      uint64_t num = setting.selection();
      num = num << uint8_t(64 - bits - i);
      num = num >> uint8_t(64 - bits);

      for (auto q : sub.metadata().enum_map())
        menu->addItem(QString::fromStdString(q.second), QVariant::fromValue(q.first));
      int menuIndex = menu->findText(QString::fromStdString(sub.metadata().enum_name(num)));
      if(menuIndex >= 0)
        menu->setCurrentIndex(menuIndex);
      vl_checks->addWidget(menu);
    }

    label = new QLabel();
    label->setFixedHeight(25);
    if (setting_.metadata().enum_map().count(i) && !is_branch)
      label->setText(QString::fromStdString(setting_.metadata().enum_name(i)));
    else if (is_branch)
      label->setText(QString::fromStdString(sub.metadata().get_string("name","")));
    else
      label->setText("N/A");
    label->setVisible(visible);
    vl_descr->addWidget(label);

    if (exists)
    {
      line = new QFrame();
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      line->setFixedHeight(3);
      line->setLineWidth(1);
      vl_bit->addWidget(line);


      line = new QFrame();
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      line->setFixedHeight(3);
      line->setLineWidth(1);
      vl_checks->addWidget(line);

      line = new QFrame();
      line->setFrameShape(QFrame::HLine);
      line->setFrameShadow(QFrame::Sunken);
      line->setFixedHeight(3);
      line->setLineWidth(1);
      vl_descr->addWidget(line);
    }

  }

  QHBoxLayout *hl = new QHBoxLayout();
  hl->addLayout(vl_bit);
  hl->addLayout(vl_checks);
  hl->addLayout(vl_descr);

  label = new QLabel();
  label->setText(QString::fromStdString("<b>" + setting_.id() + ": " +
                                        setting_.metadata().get_string("name", "")
                                        + "</b>"));

  line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setFixedHeight(3);
  line->setLineWidth(1);

  QVBoxLayout *total    = new QVBoxLayout();
  total->addWidget(label);
  total->addWidget(line);
  total->addLayout(hl);

  if (!setting.metadata().has_flag("readonly"))
  {
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(change_setting()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    total->addWidget(buttonBox);
  }
  else
  {
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(reject()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    total->addWidget(buttonBox);
  }

  setLayout(total);
}

void BinaryChecklist::change_setting()
{
  std::bitset<64> bs;
  for (size_t i=0; i < boxes_.size(); ++i)
  {
    if (boxes_[i]->isVisible())
      bs[i] = boxes_[i]->isChecked();
  }
  setting_.select(bs.to_ullong());

  for (size_t i=0; i < spins_.size(); ++i)
    if (spins_[i]->isVisible() && spins_[i]->isEnabled())
    {
      double dbl = (spins_[i]->value() - spins_[i]->minimum()) / spins_[i]->singleStep();
      //      DBG << "converted dbl " << spins_[i]->value() << " to " << dbl;
      int64_t num = static_cast<int64_t>(dbl);
      num = num << i;
      setting_.select(setting_.selection() | num);
    }

  for (size_t i=0; i < menus_.size(); ++i)
    if (menus_[i]->isVisible() && menus_[i]->isEnabled())
    {
      int64_t num = menus_[i]->currentData().toInt();
      num = num << i;
      setting_.select(setting_.selection() | num);
    }

  accept();
}



