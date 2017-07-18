#pragma once

#include <QWidget>
#include <project.h>
#include "QPlot2D.h"
#include "QPlotAppearance.h"
#include "widget_selector.h"
//#include <unordered_map>

namespace Ui {
class FormPlot2D;
}

class FormPlot2D : public QWidget
{
  Q_OBJECT

public:
  explicit FormPlot2D(QWidget *parent = 0);
  ~FormPlot2D();

  void setSpectra(DAQuiri::ProjectPtr new_set);

  void updateUI();

  void update_plot(bool force = false);
  void refresh();
  void replot_markers();
  void reset_content();

  void set_scale_type(QString);
  void set_gradient(QString);
  void set_show_legend(bool);
  QString scale_type();
  QString gradient();
  bool show_legend();

  void set_zoom(double);
  double zoom();

  int gate_width();
  void set_gate_width(int);
  void set_gates_visible(bool vertical, bool horizontal, bool diagonal);
  void set_gates_movable(bool);

signals:
  void requestAnalysis(int64_t idx);
  void requestSymmetrize(int64_t idx);

private slots:
  void spectrumDetailsDelete();

  void on_pushDetails_clicked();

  void analyse();

  void choose_spectrum(SelectorItem item);
  void crop_changed();
  void spectrumDoubleclicked(SelectorItem item);


private:
  //gui stuff
  Ui::FormPlot2D *ui;
  DAQuiri::ProjectPtr mySpectra;
  int64_t current_spectrum_;

  //plot identity
  double zoom_2d, new_zoom;
  uint32_t adjrange;

  //scaling
  int bits;

  QMenu *crop_menu_;
  QLabel *crop_label_;
  QSlider *crop_slider_;

  Container<DAQuiri::Detector> dummy_;
};
