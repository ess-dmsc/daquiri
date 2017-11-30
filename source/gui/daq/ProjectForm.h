#pragma once

#include <QWidget>
#include <QMenu>
#include "project.h"
#include "ThreadRunner.h"
#include "ThreadPlotSignal.h"

namespace Ui {
class ProjectForm;
}

class ProjectForm : public QWidget
{
    Q_OBJECT

  public:
    explicit ProjectForm(ThreadRunner&,
                         Container<DAQuiri::Detector>&,
                         std::vector<DAQuiri::Detector>&,
                         DAQuiri::ProjectPtr,
                         QWidget *parent = 0);

    //  void replot();

    ~ProjectForm();

  signals:
    void toggleIO(bool);
    void requestClose(QWidget*);

  protected:
    void closeEvent(QCloseEvent*);

  public slots:
    void toggle_push(bool, DAQuiri::ProducerStatus);
    void start_DAQ();

  private slots:
    void on_pushStart_clicked();
    void on_pushStop_clicked();
    void run_completed();
    void on_pushEditSpectra_clicked();
    void update_plots();
    void clearGraphs();

    void newProject();

    void on_pushForceRefresh_clicked();

    //  void on_pushDetails_clicked();

    void on_toggleIndefiniteRun_clicked();

    void projectSave();
    void projectSaveAs();
    void projectOpen();

    void on_doubleSpinMinPause_editingFinished();

  private:
    Ui::ProjectForm *ui;
    ThreadRunner &runner_thread_;
    DAQuiri::Interruptor interruptor_;
    DAQuiri::ProjectPtr  project_;
    bool my_run_ {false};
    bool close_me_ {false};

    Container<DAQuiri::Detector> &detectors_;
    std::vector<DAQuiri::Detector> &current_dets_;

    QString data_directory_;    //data directory

    Container<DAQuiri::ConsumerMetadata>  spectra_templates_;
    ThreadPlotSignal                plot_thread_;

    QMenu  menuLoad;
    QMenu  menuSave;

    void loadSettings();
    void saveSettings();
};
