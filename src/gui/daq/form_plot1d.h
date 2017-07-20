#pragma once

#include <QWidget>
#include <QMenu>
#include <project.h>
#include "SelectorWidget.h"
#include "AbstractConsumerWidget.h"

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

  void reset_content();

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

  void on_pushTile_clicked();

private:

  Ui::FormPlot1D *ui;

  Container<DAQuiri::Detector> detectors_;

  DAQuiri::ProjectPtr project_;

  QMap<int64_t, AbstractConsumerWidget*> spectra_;

  SelectorWidget *spectraSelector;

  QMenu menuColors;
  QMenu menuDelete;

  bool nonempty_{false};
};
