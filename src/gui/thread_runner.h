#pragma once

#include <QThread>
#include <QMutex>
#include <QVector>
#include <vector>
#include <string>
#include <cstdint>
#include <boost/atomic.hpp>

#include "engine.h"
#include "project.h"

using namespace DAQuiri;

enum RunnerAction {
  kBoot, kShutdown, kPushSettings, kSetSetting,
  kSetDetector, kSetDetectors, kList, kMCA, kOscil,
  kInitialize, kSettingsRefresh, kOptimize, kTerminate, kNone
  };

class ThreadRunner : public QThread
{
  Q_OBJECT
public:
  explicit ThreadRunner(QObject *parent = 0);
  ~ThreadRunner();

  void set_idle_refresh(bool);
  void set_idle_refresh_frequency(int);


  void do_initialize();
  void do_boot();
  void do_shutdown();
  void do_push_settings(const Setting &tree);
  void do_set_setting(const Setting &item, Match match);
  void do_set_detector(int, Detector);
  void do_set_detectors(std::map<int, Detector>);

  void do_list(boost::atomic<bool>&, uint64_t timeout);
  void do_run(ProjectPtr, boost::atomic<bool>&, uint64_t timeout);

  void do_optimize();
  void do_oscil();
  void do_refresh_settings();
  void terminate();
  bool terminating();
  bool running() {return running_.load();}

signals:
  void bootComplete();
  void runComplete();
  void listComplete(ListData);
  void settingsUpdated(Setting, std::vector<DAQuiri::Detector>, DAQuiri::ProducerStatus);
  void oscilReadOut(OscilData);

protected:
  void run();

private:
  Engine &engine_;
  QMutex mutex_;
  RunnerAction action_;
  boost::atomic<bool> running_;
  boost::atomic<bool> idle_refresh_;
  boost::atomic<int> idle_refresh_frequency_;


  ProjectPtr spectra_;
  boost::atomic<bool>* interruptor_;
  boost::atomic<bool> terminating_;

  uint64_t timeout_;

  std::map<int, Detector> detectors_;
  Detector det_;
  int chan_;
  Setting tree_, one_setting_;
  Match match_conditions_;

  bool flag_;

  DAQuiri::ProducerStatus recent_status_;
};

Setting get_profile();
