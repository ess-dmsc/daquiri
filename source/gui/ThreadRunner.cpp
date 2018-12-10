#include <gui/ThreadRunner.h>
#include <gui/Profiles.h>

#include <core/util/custom_logger.h>

ThreadRunner::ThreadRunner(QObject *parent)
  : QThread(parent)
  , engine_(Engine::singleton())
  , running_(false)
  , terminating_(false)
{
  idle_refresh_.store(false);
  idle_refresh_frequency_.store(1);
  start(HighPriority);
}

ThreadRunner::~ThreadRunner()
{}

void ThreadRunner::terminate()
{
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
    WARN("Runner busy");
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
    WARN("Runner busy");
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


void ThreadRunner::remove_producer(const Setting& node)
{
  if (running_.load())
  {
    WARN("Runner busy");
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  one_setting_ = node;
  action_ = kRemoveProducer;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::add_producer(const Setting& node)
{
  if (running_.load())
  {
    WARN("Runner busy");
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  one_setting_ = node;
  action_ = kAddProducer;
  if (!isRunning())
    start(HighPriority);
}

void ThreadRunner::do_initialize(QString profile_name,
                                 bool and_boot)
{
  if (running_.load())
  {
    WARN("Runner busy");
    return;
  }
  QMutexLocker locker(&mutex_);
  terminating_.store(false);
  profile_name_ = profile_name;
  action_ = kChooseProfile;
  and_boot_ = and_boot;
  if (!isRunning())
    start(HighPriority);
}


void ThreadRunner::do_boot()
{
  if (running_.load())
  {
    WARN("Runner busy");
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
    WARN("Runner busy");
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
    WARN("Runner busy");
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
    WARN("Runner busy");
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

void ThreadRunner::do_oscil()
{
  if (running_.load())
  {
    WARN("Runner busy");
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
    WARN("Runner busy");
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
      emit settingsUpdated(engine_.settings(),
                           status_before_run(),
                           engine_.stream_manifest());
      interruptor_->store(false);
      engine_.acquire(project_, *interruptor_, timeout_);
      action_ = kSettingsRefresh;
      emit runComplete();
    }
    else if (action_ == kList)
    {
      interruptor_->store(false);
      emit settingsUpdated(engine_.settings(),
                           status_before_run(),
                           engine_.stream_manifest());
      ListData newListRun
          = engine_.acquire_list(*interruptor_, timeout_);
      action_ = kSettingsRefresh;
      emit listComplete(newListRun);
    }
    else if (action_ == kChooseProfile)
    {
      save_profile();
      Profiles::singleton().select_profile(profile_name_, and_boot_);
      engine_.initialize(Profiles::singleton().get_profile(profile_name_));
      if (and_boot_)
        action_ = kBoot;
      else
        action_ = kSettingsRefresh;
    }
    else if (action_ == kAddProducer)
    {
      engine_.get_all_settings();
      auto tree = engine_.settings();
      tree.branches.add_a(one_setting_);
      engine_.initialize(tree);
      action_ = kSettingsRefresh;
    }
    else if (action_ == kRemoveProducer)
    {
      engine_.get_all_settings();
      auto tree = engine_.settings();
      tree.erase(one_setting_, Match::id | Match::value);
      engine_.initialize(tree);
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
    else if (action_ == kPushSettings)
    {
      engine_.settings(tree_);
      action_ = kSettingsRefresh;
    }
    else if (action_ == kSetSetting)
    {
      engine_.set_setting(tree_, match_conditions_);
      action_ = kSettingsRefresh;
    }
    else if (action_ == kOscil)
    {
      auto traces = engine_.oscilloscope();
      if (!traces.empty())
        emit oscilReadOut(traces);
      action_ = kSettingsRefresh;
    }
    else if (action_ == kSettingsRefresh)
    {
      engine_.get_all_settings();
      action_ = kNone;
      emit settingsUpdated(engine_.settings(),
                           engine_.status(),
                           engine_.stream_manifest());
    }
    else
    {
      bool booted = ((engine_.status()
                      & DAQuiri::ProducerStatus::booted) != 0);
      if (booted && idle_refresh_.load())
      {
        action_ = kSettingsRefresh;
        QThread::sleep(idle_refresh_frequency_.load());
        //DBG( "idle runner will refresh settings";
      }
    }
    running_.store(false);
  }

  save_profile();
}

DAQuiri::ProducerStatus ThreadRunner::status_before_run()
{
  return (engine_.status() ^ DAQuiri::ProducerStatus::can_run)
      | DAQuiri::ProducerStatus::running;
}

void ThreadRunner::save_profile()
{
  engine_.die();
  engine_.get_all_settings();
  auto dev_settings = engine_.settings();
  dev_settings.condense();
  dev_settings.strip_metadata();
  if (dev_settings)
    Profiles::singleton().save_profile(dev_settings);
}
