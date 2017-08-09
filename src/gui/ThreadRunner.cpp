#include <boost/thread.hpp>
#include <QSettings>
#include <QDir>
#include "ThreadRunner.h"
#include "custom_logger.h"

#include "mock_producer.h"
#include "json_file.h"
#include <boost/filesystem.hpp>

ThreadRunner::ThreadRunner(QObject *parent) :
  QThread(parent),
  engine_(Engine::singleton()),
  terminating_(false),
  running_(false)
{
  spectra_ = nullptr;
  interruptor_ = nullptr;
  action_ = kNone;
  flag_ = false;
  match_conditions_ = Match::id;
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


void ThreadRunner::do_list(boost::atomic<bool> &interruptor, uint64_t timeout)
{
  if (running_.load()) {
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


void ThreadRunner::do_run(ProjectPtr spectra, boost::atomic<bool> &interruptor, uint64_t timeout)
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  spectra_ = spectra;
  interruptor_ = &interruptor;
  timeout_ = timeout;
  action_ = kMCA;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_initialize()
{
  if (running_.load())
  {
    WARN << "Runner busy";
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  action_ = kInitialize;
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

    if (action_ == kMCA)
    {
      engine_.get_all_settings();
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(),
                           engine_.status() ^ DAQuiri::ProducerStatus::can_run); //turn off can_run
      interruptor_->store(false);
      engine_.acquire(spectra_, *interruptor_, timeout_);
      action_ = kSettingsRefresh;
      emit runComplete();
    }
    else if (action_ == kList)
    {
      interruptor_->store(false);
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(),
                           engine_.status() ^ DAQuiri::ProducerStatus::can_run); //turn off can_run
      ListData newListRun = engine_.acquire_list(*interruptor_, timeout_);
      action_ = kSettingsRefresh;
      emit listComplete(newListRun);
    }
    else if (action_ == kInitialize)
    {
      QSettings settings;
      settings.beginGroup("Program");
//      QString settings_directory = settings_.value("profile_directory", QDir::homePath() + "/daquiri/settings").toString();
      QString profile_directory = settings.value("profile_directory", QDir::homePath() + "/daquiri/settings").toString();
      bool boot = settings.value("boot_on_startup", false).toBool();

      auto path = profile_directory.toStdString() + "/profile.set";

      DBG << "Will load from " << path;

      json profile;
      if (boost::filesystem::exists(path))
        profile = from_json_file(path);
      if (profile.empty())
        profile = get_profile();

      json defs;
      defs["MockProducer"] = json();

//      engine_.initialize(profile_directory.toStdString() + "/profile.set", settings_directory.toStdString());

      engine_.initialize(profile, defs);

      if (boot)
      {
        action_ = kBoot;
      } else {
        action_ = kNone;
        emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(), engine_.status());
      }
    }
    else if (action_ == kBoot)
    {
      engine_.boot();
      engine_.get_all_settings();
      action_ = kNone;
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(), engine_.status());
      emit bootComplete();
    }
    else if (action_ == kShutdown)
    {
      engine_.die();
      engine_.get_all_settings();
      action_ = kNone;
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(), engine_.status());
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
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(), engine_.status());
    }
    else if (action_ == kPushSettings)
    {
      engine_.push_settings(tree_);
      engine_.get_all_settings();
      action_ = kNone;
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(), engine_.status());
    }
    else if (action_ == kSetSetting)
    {
      engine_.set_setting(tree_, match_conditions_);
      engine_.get_all_settings();
      action_ = kNone;
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(), engine_.status());
    }
    else if (action_ == kSetDetector)
    {
      engine_.set_detector(chan_, det_);
      engine_.write_settings_bulk();
      engine_.get_all_settings();
      action_ = kNone;
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(), engine_.status());
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
      engine_.get_all_settings();
      action_ = kNone;
      if (!traces.empty())
        emit oscilReadOut(traces);
      emit settingsUpdated(engine_.pull_settings(), engine_.get_detectors(), engine_.status());
    }
    else
    {
      bool booted = ((engine_.status() & DAQuiri::ProducerStatus::booted) != 0);
      if (booted && idle_refresh_.load())
      {
        action_ = kSettingsRefresh;
        QThread::sleep(idle_refresh_frequency_.load());
        //DBG << "idle runner will refresh settings";
      }
    }
    running_.store(false);
  }

  QSettings settings;
  settings.beginGroup("Program");
  QString profile_directory = settings.value("profile_directory",
                                             QDir::homePath() + "/daquiri/settings").toString();


  auto path = profile_directory.toStdString() + "/profile.set";

  DBG << "Will save to " << path;

  auto dev_settings = DAQuiri::Engine::singleton().pull_settings();
  dev_settings.condense();
  dev_settings.strip_metadata();
  to_json_file(dev_settings, path);

}



Setting get_profile()
{
  Setting default_settings({MockProducer().device_name(),
                                     SettingType::stem});
  MockProducer dummy;
  dummy.write_settings_bulk(default_settings);
  dummy.read_settings_bulk(default_settings);

  default_settings.set(Setting::integer("MockProducer/SpillInterval", 5));
  default_settings.set(Setting::integer("MockProducer/Resolution", 16));
  default_settings.set(Setting::floating("MockProducer/CountRate", 20000));
  default_settings.set(Setting::floating("MockProducer/DeadTime", 5));

  default_settings.set(Setting::integer("MockProducer/ValCount", 3));

  auto profile = Engine::singleton().pull_settings();
  profile.set(Setting::text("Profile description",
                            "Test profile for Mock Producer"));
  profile.branches.add(default_settings);

  return profile;
}
