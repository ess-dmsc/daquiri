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
  kSetDetector, kSetDetectors, kList, kAcquire, kOscil,
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


  void do_initialize(const json &profile, bool and_boot);
  void do_boot();
  void do_shutdown();
  void do_push_settings(const Setting &tree);
  void do_set_setting(const Setting &item, Match match);
  void do_set_detector(int, Detector);
  void do_set_detectors(std::map<int, Detector>);

  void do_list(Interruptor &, uint64_t timeout);
  void do_run(ProjectPtr, Interruptor &, uint64_t timeout);

  void do_optimize();
  void do_oscil();
  void do_refresh_settings();
  void terminate();
  bool terminating();
  bool running() {return running_.load();}

signals:
  void bootComplete();
  void runComplete();
  void listComplete(DAQuiri::ListData);
  void settingsUpdated(DAQuiri::Setting,
                       std::vector<DAQuiri::Detector>,
                       DAQuiri::ProducerStatus);
  void oscilReadOut(DAQuiri::OscilData);

protected:
  void run();

private:
  Engine &engine_;
  QMutex mutex_;
  RunnerAction action_ {kNone};
  boost::atomic<bool> running_;
  boost::atomic<bool> idle_refresh_;
  boost::atomic<int> idle_refresh_frequency_;


  ProjectPtr project_;
  DAQuiri::Interruptor* interruptor_ {nullptr};
  DAQuiri::Interruptor  terminating_;

  uint64_t timeout_;

  std::map<int, Detector> detectors_;
  Detector det_;
  int chan_;
  Setting tree_, one_setting_;
  Match match_conditions_ {Match::id};

  json profile_;
  bool and_boot_ {false};

  DAQuiri::ProducerStatus recent_status_;


  void save_profile();
};

Setting default_profile();
