#pragma once

#include <QPushButton>

namespace Ui {
class daquiri;
}

class close_tab_widgetButton : public QPushButton {
  Q_OBJECT
public:
  close_tab_widgetButton(QWidget *pt) : parent_tab_(pt) {
    connect(this, SIGNAL(clicked(bool)), this, SLOT(closeme(bool)));
  }
private slots:
  void closeme(bool) { emit close_tab_widget(parent_tab_); }
signals:
  void close_tab_widget(QWidget *);

protected:
  QWidget *parent_tab_;
};
