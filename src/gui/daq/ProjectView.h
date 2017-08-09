#pragma once

#include <QWidget>
#include <QMenu>
#include <project.h>
#include "SelectorWidget.h"
#include "AbstractConsumerWidget.h"

namespace Ui {
class ProjectView;
}

class ProjectView : public QWidget
{
  Q_OBJECT

public:
  explicit ProjectView(QWidget *parent = 0);
  ~ProjectView();

  void setSpectra(DAQuiri::ProjectPtr new_set);

  void updateUI();

  void update_plots();

private slots:
  void selectorItemToggled(SelectorItem);
  void selectorItemSelected(SelectorItem);
  void selectorItemDoubleclicked(SelectorItem);

  void on_pushFulINFO_clicked();

  void showAll();
  void hideAll();
  void randAll();

  void deleteSelected();
  void deleteShown();
  void deleteHidden();

  void tile_grid();
  void tile_horizontal();
  void tile_vertical();

  void consumerWidgetDestroyed(QObject*);

private:
  Ui::ProjectView *ui;

  Container<DAQuiri::Detector> detectors_;

  DAQuiri::ProjectPtr project_;

  QMap<int64_t, AbstractConsumerWidget*> consumers_;

  SelectorWidget *selector_;

  QMenu colors_menu_;
  QMenu delete_menu_;
  QMenu tile_menu_;

  void enforce_item(SelectorItem);
  void enforce_all();
};
