#pragma once

#include <core/project.h>

#include <gui/widgets/SelectorWidget.h>
#include <gui/daq/AbstractConsumerWidget.h>

#include <QWidget>
#include <QMenu>
#include <QMdiArea>

namespace Ui
{
class ProjectView;
}

class ConsumerMulti1D;

class ProjectView : public QWidget
{
 Q_OBJECT

 public:
  explicit ProjectView(QWidget* parent = 0);
  ~ProjectView();

  void set_project(DAQuiri::ProjectPtr project);

  void set_manifest(DAQuiri::StreamManifest);

  void updateUI();

  void update_plots();

//  protected:
//    void closeEvent(QCloseEvent*);

 private slots:
  void selectorItemToggled(SelectorItem);
  void selectorItemSelected(SelectorItem);
  void selectorItemDoubleclicked(SelectorItem);

  void on_pushFullInfo_clicked();

  void showAll();
  void hideAll();
  void randAll();

  void deleteSelected();
  void deleteShown();
  void deleteHidden();

  void tile_free();
  void tile_grid();
  void tile_horizontal();
  void tile_vertical();

  void consumerWidgetDestroyed(QObject*);
  void groupWidgetDestroyed(QObject*);

  void on_pushHideControls_clicked();

  void enforce_tile_policy();

 private:
  Ui::ProjectView* ui;

  Container<DAQuiri::Detector> detectors_;
  DAQuiri::StreamManifest stream_manifest_;

  DAQuiri::ProjectPtr project_;

  // consumer_id->widget
  QMap<int64_t, AbstractConsumerWidget*> consumers_;

  // window_group->widget
  QMap<size_t, ConsumerMulti1D*> groups_;

  SelectorWidget* selector_;

  QString tile_policy_{"grid"};

  QMenu colors_menu_;
  QMenu delete_menu_;
  QMenu tile_menu_;

  void enforce_all();

  static void tile_grid(QMdiArea*);
  static void tile_horizontal(QMdiArea*);
  static void tile_vertical(QMdiArea*);

  void loadSettings();
  void saveSettings();
};
