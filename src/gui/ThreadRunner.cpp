#include <boost/thread.hpp>
#include <QSettings>
#include <QDir>
#include "ThreadRunner.h"
#include "custom_logger.h"

#include "Profiles.h"

ThreadRunner::ThreadRunner(QObject *parent) :
  QThread(parent),
  engine_(Engine::singleton()),
  terminating_(false),
  running_(false)
{
  idle_refresh_.store(false);
  idle_refresh_frequency_.store(1);
  start(HighPriority);
}

ThreadRunner::~ThreadRunner()
{}

void ThreadRunner::terminate()
{
  //  INFO << "runner thread termination requested";
  if (interruptor_)
    interruptor_->store(true);
  terminating_.store(true);
  wait();
}

bool ThreadRunner::terminating()
{
  return terminating_.load();
}

void ThreadRunner::set_idle_refresh(bool refresh)
{
  idle_refresh_.store(refresh);
}

void ThreadRunner::set_idle_refresh_frequency(int secs)
{
  if (secs < 1)
    secs = 1;
  idle_refresh_frequency_.store(secs);
}


void ThreadRunner::do_list(DAQuiri::Interruptor& interruptor, uint64_t timeout)
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  interruptor_ = &interruptor;
  timeout_ = timeout;
  action_ = kList;
  if (!isRunning())
    start(HighPriority);
}


void ThreadRunner::do_run(ProjectPtr spectra, DAQuiri::Interruptor& interruptor, uint64_t timeout)
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  project_ = spectra;
  interruptor_ = &interruptor;
  timeout_ = timeout;
  action_ = kAcquire;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_initialize(const json& profile,
                                 bool and_boot)
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  profile_ = profile;
  action_ = kInitialize;
  and_boot_ = and_boot;
  if (!isRunning())
    start(HighPriority);
}


void ThreadRunner::do_boot()
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  action_ = kBoot;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_shutdown()
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  action_ = kShutdown;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_push_settings(const Setting &tree)
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  tree_ = tree;
  action_ = kPushSettings;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_set_setting(const Setting &item, Match match)
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  tree_ = item;
  match_conditions_ = match;
  action_ = kSetSetting;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_set_detector(int chan, Detector det)
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  chan_ = chan;
  det_ = det;
  action_ = kSetDetector;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_set_detectors(std::map<int, Detector> dets)
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  detectors_ = dets;
  action_ = kSetDetectors;
  if (!isRunning())
    start(HighPriority);
}


void ThreadRunner::do_optimize()
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  action_ = kOptimize;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_oscil()
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  action_ = kOscil;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_refresh_settings()
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  action_ = kSettingsRefresh;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::run()
{
  while (!terminating_.load())
  {

    if (action_ != kNone)
      running_.store(true);

    if (action_ == kAcquire)
    {
      engine_.get_all_settings();
      emit settingsUpdated(engine_.pull_settings(),
                           engine_.get_detectors(),
                           engine_.status()
                           ^ DAQuiri::ProducerStatus::can_run
                           | DAQuiri::ProducerStatus::running);
      interruptor_->store(false);
      engine_.acquire(project_, *interruptor_, timeout_);
      action_ = kSettingsRefresh;
      emit runComplete();
    }
    else if (action_ == kList)
    {
      interruptor_->store(false);
      emit settingsUpdated(engine_.pull_settings(),
                           engine_.get_detectors(),
                           engine_.status()
                           ^ DAQuiri::ProducerStatus::can_run
                           | DAQuiri::ProducerStatus::running);
      ListData newListRun
          = engine_.acquire_list(*interruptor_, timeout_);
      action_ = kSettingsRefresh;
      emit listComplete(newListRun);
    }
    else if (action_ == kInitialize)
    {
      engine_.initialize(profile_);
      if (and_boot_)
        action_ = kBoot;
      else
        action_ = kSettingsRefresh;
    }
    else if (action_ == kBoot)
    {
      engine_.boot();
      emit bootComplete();
      action_ = kSettingsRefresh;
    }
    else if (action_ == kShutdown)
    {
      engine_.die();
      action_ = kSettingsRefresh;
    }
    else if (action_ == kOptimize)
    {
      engine_.load_optimizations();
      action_ = kOscil;
    }
    else if (action_ == kSettingsRefresh)
    {
      engine_.get_all_settings();
      action_ = kNone;
      emit settingsUpdated(engine_.pull_settings(),
                           engine_.get_detectors(),
                           engine_.status());
    }
    else if (action_ == kPushSettings)
    {
      engine_.push_settings(tree_);
      action_ = kSettingsRefresh;
    }
    else if (action_ == kSetSetting)
    {
      engine_.set_setting(tree_, match_conditions_);
      action_ = kSettingsRefresh;
    }
    else if (action_ == kSetDetector)
    {
      engine_.set_detector(chan_, det_);
      engine_.write_settings_bulk();
      action_ = kSettingsRefresh;
    }
    else if (action_ == kSetDetectors)
    {
      for (auto &q : detectors_)
        engine_.set_detector(q.first, q.second);
      engine_.load_optimizations();
      engine_.write_settings_bulk();
      action_ = kOscil;
    }
    else if (action_ == kOscil)
    {
      auto traces = engine_.oscilloscope();
      if (!traces.empty())
        emit oscilReadOut(traces);
      action_ = kSettingsRefresh;
    }
    else
    {
      bool booted = ((engine_.status()
                      & DAQuiri::ProducerStatus::booted) != 0);
      if (booted && idle_refresh_.load())
      {
        action_ = kSettingsRefresh;
        QThread::sleep(idle_refresh_frequency_.load());
        //DBG << "idle runner will refresh settings";
      }
    }
    running_.store(false);
  }

  save_profile();
}

void ThreadRunner::save_profile()
{
  engine_.die();
  engine_.get_all_settings();
  auto dev_settings = engine_.pull_settings();
  dev_settings.condense();
  dev_settings.strip_metadata();
  Profiles::save_profile(dev_settings);
}
