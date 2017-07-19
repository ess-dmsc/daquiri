#pragma once

#include <QWidget>
#include "ThreadRunner.h"
#include "SelectorWidget.h"

namespace Ui {
class FormOscilloscope;
}

class FormOscilloscope : public QWidget
{
  Q_OBJECT

public:
  explicit FormOscilloscope(QWidget *parent = 0);
  ~FormOscilloscope();

signals:
  void refresh_oscil();

protected:
  void closeEvent(QCloseEvent*);

public slots:
  void oscil_complete(DAQuiri::OscilData);
  void toggle_push(bool, DAQuiri::ProducerStatus);

private slots:
  void on_pushOscilRefresh_clicked();

  void channelToggled(SelectorItem);
  void channelDetails(SelectorItem);
  void on_pushShowAll_clicked();
  void on_pushHideAll_clicked();

public slots:
  void updateMenu(std::vector<DAQuiri::Detector>);
  
private:
  Ui::FormOscilloscope *ui;
  OscilData traces_;
  std::vector<Detector> channels_;

  void replot();
};
