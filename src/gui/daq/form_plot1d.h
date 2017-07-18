#pragma once

#include <QWidget>
#include <project.h>
#include "widget_selector.h"
#include "QPlot1D.h"

namespace Ui {
class FormPlot1D;
}

class FormPlot1D : public QWidget
{
  Q_OBJECT

public:
  explicit FormPlot1D(QWidget *parent = 0);
  ~FormPlot1D();

  void setSpectra(DAQuiri::ProjectPtr new_set);

  void updateUI();

  void update_plot();

  void replot_markers();
  void reset_content();

  void set_scale_type(QString);
  void set_plot_style(QString);
  QString scale_type();
  QString plot_style();

private slots:
  void spectrumDetailsDelete();

  void spectrumLooksChanged(SelectorItem);
  void spectrumDetails(SelectorItem);
  void spectrumDoubleclicked(SelectorItem);

  void on_pushFullInfo_clicked();

  void showAll();
  void hideAll();
  void randAll();

  void deleteSelected();
  void deleteShown();
  void deleteHidden();

private:

  Ui::FormPlot1D *ui;

  Container<DAQuiri::Detector> detectors_;

  DAQuiri::ProjectPtr mySpectra;

  SelectorWidget *spectraSelector;

  QMenu menuColors;
  QMenu menuDelete;
  QMenu menuEffCal;

  bool nonempty_{false};

};
