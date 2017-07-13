#include "engine.h"
#include "custom_logger.h"
#include "producer_factory.h"
#include "custom_timer.h"

//#include "core_compiletime.h"

namespace DAQuiri {

//int Engine::print_version()
//{
//  LINFO << "<DAQuiri> " << Engine::version();
//}

//std::string Engine::version()
//{
//  return "git.SHA1=" + std::string(GIT_VERSION)
//      + " compiled at " + std::string(_TIMEZ_)
//      + " on " + std::string(CMAKE_SYSTEM)
//      + " with " + std::string(CMAKE_SYSTEM_PROCESSOR);
//}

//const static int initializer = Engine::print_version();

Engine::Engine()
{
  total_det_num_ = SettingMeta("Total detectors", SettingType::integer);
  total_det_num_.set_val("min", 1);
  total_det_num_.set_val("max", 64);
}

void Engine::initialize(const json &profile, const json &definitions)
{
  Setting tree = profile;

  auto& pf = ProducerFactory::singleton();

  die();
  devices_.clear();
  for (auto &q : tree.branches)
  {
    if (q.id() != "Detectors")
    {
//      DBG << "Will try to load " << q.get_text();
      if (!definitions.count(q.id()))
        continue;
      ProducerPtr device = pf.create_type(q.id(), definitions.at(q.id()));
      if (device)
      {
        DBG << "<Engine> Success loading " << device->device_name();
        devices_[q.id()] = device;
      }
    }
  }

  push_settings(tree);
  get_all_settings();

  Setting descr = tree.find(Setting("Profile description"));
  LINFO << "<Engine> Welcome to " << descr.get_text();
}

Engine::~Engine()
{
  die();

//    Setting dev_settings = pull_settings();
//    dev_settings.condense();
//    dev_settings.strip_metadata();
}

void Engine::boot()
{
  bool success = false;
  for (auto &q : devices_)
    if ((q.second != nullptr) &&
        (q.second->status() & ProducerStatus::can_boot))
    {
      success |= q.second->boot();
      //DBG << "daq_start > " << q.second->device_name();
    }

  if (success)
  {
    LINFO << "<Engine> Boot successful";
    //settings_tree_.value_int = 2;
    get_all_settings();
  } else
    LINFO << "<Engine> Boot failed";
}

void Engine::die()
{
  for (auto &q : devices_)
    if ((q.second != nullptr) &&
        (q.second->status() & ProducerStatus::booted))
    {
      q.second->die();
      //DBG << "die > " << q.second->device_name();
    }

  get_all_settings();
}

Setting Engine::pull_settings() const
{
  return settings_tree_;
}

void Engine::push_settings(const Setting& newsettings)
{
  settings_tree_ = newsettings;
  write_settings_bulk();

//  LINFO << "settings pushed branches = " << settings_tree_.branches.size();
}

void Engine::read_settings_bulk()
{
  for (auto &set : settings_tree_.branches)
  {
    if (set.id() == "Detectors")
    {
      Setting totaldets(total_det_num_);
      totaldets.set_number(detectors_.size());

      set.branches.clear();
      set.branches.add_a(totaldets);

      for (size_t i=0; i < detectors_.size(); ++i)
      {
        Setting det(SettingMeta("Detector",
                                SettingType::detector,
                                "Detector " + std::to_string(i)));
        det.set_text(detectors_[i].name());
        det.set_indices({int32_t(i)});
        set.branches.add_a(det);
      }

    }
    else if (devices_.count(set.id()))
    {
      //DBG << "read settings bulk > " << set.id_;
      devices_[set.id()]->read_settings_bulk(set);
    }
  }
  save_optimization();
}

void Engine::write_settings_bulk()
{
  for (auto &set : settings_tree_.branches)
  {
    if (set.id() == "Detectors")
      rebuild_structure(set);
    else if (devices_.count(set.id()))
      devices_[set.id()]->write_settings_bulk(set);
  }
}

void Engine::rebuild_structure(Setting &set)
{
  Setting totaldets = set.find({"Total detectors"});
  int oldtotal = detectors_.size();
  int newtotal = totaldets.get_number();
  if (newtotal < 0)
    newtotal = 0;

  if (oldtotal != newtotal)
    detectors_.resize(newtotal);
}

std::vector<Event> Engine::oscilloscope()
{
  std::vector<Event> traces;
  traces.resize(detectors_.size());

  for (auto &q : devices_)
    if ((q.second != nullptr) && (q.second->status() & ProducerStatus::can_oscil))
    {
      //DBG << "oscil > " << q.second->device_name();
      std::list<Event> trc = q.second->oscilloscope();
      for (auto &p : trc)
        if ((p.channel() >= 0) && (p.channel() < static_cast<int16_t>(traces.size())))
          traces[p.channel()] = p;
    }
  return traces;
}

bool Engine::daq_start(SpillQueue out_queue)
{
  bool success = false;
  for (auto &q : devices_)
    if ((q.second != nullptr) && (q.second->status() & ProducerStatus::can_run))
    {
      success |= q.second->daq_start(out_queue);
      //DBG << "daq_start > " << q.second->device_name();
    }
  return success;
}

bool Engine::daq_stop()
{
  bool success = false;
  for (auto &q : devices_)
    if ((q.second != nullptr) && (q.second->status() & ProducerStatus::can_run))
    {
      success |= q.second->daq_stop();
      //DBG << "daq_stop > " << q.second->device_name();
    }
  return success;
}

bool Engine::daq_running() const
{
  bool running = false;
  for (auto &q : devices_)
    if ((q.second != nullptr) && (q.second->status() & ProducerStatus::can_run))
    {
      running |= q.second->daq_running();
      //DBG << "daq_check > " << q.second->device_name();
    }
  return running;
}

void Engine::set_detector(size_t ch, Detector det)
{
  if (ch >= detectors_.size())
    return;
  detectors_[ch] = det;
  //DBG << "set det #" << ch << " to  " << det.name_;

  for (auto &set : settings_tree_.branches)
  {
    if (set.id() == "Detectors")
    {
      for (auto &q : set.branches)
      {
        if (q.has_index(ch))
        {
          q.set_text(detectors_[ch].name());
          load_optimization(ch);
        }
      }
    }
  }
}

void Engine::save_optimization()
{
  for (size_t i = 0; i < detectors_.size(); i++)
  {
//    DBG << "Saving optimization channel " << i << " settings for " << detectors_[i].name_;
//    detectors_[i].settings_ = Setting();
    Setting t;
    t.set_indices({int32_t(i)});
    detectors_[i].add_optimizations(settings_tree_.find_all(t, Match::indices));
  }
}

void Engine::load_optimization()
{
  for (size_t i = 0; i < detectors_.size(); i++)
    load_optimization(i);
}

void Engine::load_optimization(size_t i)
{
  if (i >= detectors_.size())
    return;
  for (auto s : detectors_[i].optimizations())
  {
    if (s.metadata().has_flag("readonly"))
      continue;
    s.set_indices({int32_t(i)});
    settings_tree_.set(s, Match::id | Match::indices, true);
  }
}

void Engine::set_setting(Setting address, Match flags, bool greedy)
{
  settings_tree_.set(address, flags, greedy);
  write_settings_bulk();
  read_settings_bulk();
}

void Engine::get_all_settings()
{
  aggregate_status_ = ProducerStatus(0);
  for (auto &q : devices_)
  {
    q.second->get_all_settings();
    aggregate_status_ = aggregate_status_ | q.second->status();
  }
  read_settings_bulk();
}

void Engine::acquire(ProjectPtr spectra, Interruptor &interruptor, uint64_t timeout)
{
  boost::unique_lock<boost::mutex> lock(mutex_);

  if (!spectra)
  {
    WARN << "<Engine> No reference to valid daq project";
    return;
  }

  if (!(aggregate_status_ & ProducerStatus::can_run))
  {
    WARN << "<Engine> No devices exist that can perform acquisition";
    return;
  }

  if (timeout > 0)
    LINFO << "<Engine> Starting acquisition scheduled for " << timeout << " seconds";
  else
    LINFO << "<Engine> Starting acquisition for indefinite run";

  CustomTimer *anouncement_timer = nullptr;
  double secs_between_anouncements = 5;

  SynchronizedQueue<Spill*> parsedQueue;

  boost::thread builder(boost::bind(&Engine::worker_chronological, this, &parsedQueue, spectra));

  Spill* spill = new Spill;
  get_all_settings();
  spill->state = pull_settings();
  spill->detectors = get_detectors();
  parsedQueue.enqueue(spill);

  if (!daq_start(&parsedQueue))
    ERR << "<Engine> Failed to start device daq threads";

  CustomTimer total_timer(timeout, true);
  anouncement_timer = new CustomTimer(true);

  while (daq_running())
  {
    wait_ms(1000);
    if (anouncement_timer->s() > secs_between_anouncements)
    {
      if (timeout > 0)
        LINFO << "  RUNNING Elapsed: " << total_timer.done()
                << "  ETA: " << total_timer.ETA();
      else
        LINFO << "  RUNNING Elapsed: " << total_timer.done();

      delete anouncement_timer;
      anouncement_timer = new CustomTimer(true);
    }
    if (interruptor.load() || (timeout && total_timer.timeout()))
    {
      if (!daq_stop())
        ERR << "<Engine> Failed to stop device daq threads";
    }
  }

  delete anouncement_timer;

  spill = new Spill;
  get_all_settings();
  spill->state = pull_settings();
  parsedQueue.enqueue(spill);

  wait_ms(500);
  while (parsedQueue.size() > 0)
    wait_ms(500);
  parsedQueue.stop();
  wait_ms(500);

  builder.join();
  LINFO << "<Engine> Acquisition finished";
}

ListData Engine::acquire_list(Interruptor& interruptor, uint64_t timeout)
{
  boost::unique_lock<boost::mutex> lock(mutex_);

  if (!(aggregate_status_ & ProducerStatus::can_run))
  {
    WARN << "<Engine> No devices exist that can perform acquisition";
    return ListData();
  }

  if (timeout > 0)
    LINFO << "<Engine> List mode acquisition scheduled for " << timeout << " seconds";
  else
    LINFO << "<Engine> List mode acquisition indefinite run";

  Spill* one_spill;
  ListData result;

  CustomTimer *anouncement_timer = nullptr;
  double secs_between_anouncements = 5;

  one_spill = new Spill;
  get_all_settings();
  one_spill->state = pull_settings();
  one_spill->detectors = get_detectors();
  result.push_back(SpillPtr(one_spill));

  SynchronizedQueue<Spill*> parsedQueue;

  if (!daq_start(&parsedQueue))
    ERR << "<Engine> Failed to start device daq threads";

  CustomTimer total_timer(timeout, true);
  anouncement_timer = new CustomTimer(true);

  while (daq_running())
  {
    wait_ms(1000);
    if (anouncement_timer->s() > secs_between_anouncements)
    {
      LINFO << "  RUNNING Elapsed: " << total_timer.done()
              << "  ETA: " << total_timer.ETA();
      delete anouncement_timer;
      anouncement_timer = new CustomTimer(true);
    }
    if (interruptor.load() || (timeout && total_timer.timeout()))
    {
      if (!daq_stop())
        ERR << "<Engine> Failed to stop device daq threads";
    }
  }

  delete anouncement_timer;

  one_spill = new Spill;
  get_all_settings();
  one_spill->state = pull_settings();
  parsedQueue.enqueue(one_spill);
//  result.push_back(SpillPtr(one_spill));

  wait_ms(500);

  while (parsedQueue.size() > 0)
    result.push_back(SpillPtr(parsedQueue.dequeue()));

  parsedQueue.stop();
  return result;
}

//////STUFF BELOW SHOULD NOT BE USED DIRECTLY////////////
//////ASSUME YOU KNOW WHAT YOU'RE DOING WITH THREADS/////

void Engine::worker_chronological(SpillQueue data_queue,
                        ProjectPtr spectra)
{
  CustomTimer presort_timer;
  uint64_t presort_compares(0), presort_events(0), presort_cycles(0);

  std::map<int16_t, bool> queue_status;
  // for each input channel (detector) false = empty, true = data

  // use multimap from timestamp?
  std::list<Spill*> current_spills;

  while (true)
  {
    Spill* in_spill = data_queue->dequeue();
    if (in_spill != nullptr)
    {
//      DBG << "Spill arrived with " << in_spill->events.size();
      current_spills.push_back(in_spill);
    }

//    if (in_spill)
//      DBG << "<Engine: worker_chronological> spill backlog " << current_spills.size()
//             << " at arrival of " << boost::posix_time::to_iso_extended_string(in_spill->time);

    bool empty = false;
    while (!empty)
    {
      empty = queue_status.empty();
      if (in_spill != nullptr)
      {
        for (auto &q : queue_status)
          q.second = false;
        for (auto i : current_spills)
        {
          for (auto &q : i->stats)
          {
            if ((q.second.channel() >= 0) &&
                (!i->events.empty() || (q.second.type() == StatusType::stop) ))
              queue_status[q.second.channel()] = true;
          }
        }
      }
      else
      {
        for (auto &q : queue_status)
          q.second = true;
      }

      for (auto q : queue_status)
        if (!q.second)
          empty = true;

//      DBG << "Empty? " << empty;

      Spill* out_spill = new Spill;
      presort_cycles++;
      presort_timer.start();
      while (!empty)
      {
        Event oldest;
        for (auto &q : current_spills)
        {
          if (q->events.empty())
          {
            empty = true;
            break;
          }
          else if ((oldest == Event()) ||
                   (q->events.front().timestamp() < oldest.timestamp()))
          {
            oldest = q->events.front();
            presort_compares++;
          }
        }
        if (!empty)
        {
          presort_events++;
          out_spill->events.push_back(oldest);
          for (auto &q : current_spills)
            if ((!q->events.empty()) &&
                (q->events.front().timestamp() == oldest.timestamp()))
            {
              q->events.pop_front();
              presort_compares++;
              break;
            }
        }
        if (current_spills.empty())
          empty = true;
      }
      presort_timer.stop();

//      DBG << "<Engine> Presort pushed " << presort_events << " events, ";
//                               " time/hit: " << presort_timer.us()/presort_events << "us, "
//                               " time/cycle: " << presort_timer.us()/presort_cycles  << "us, "
//                               " compares/hit: " << double(presort_compares)/double(presort_events) << ", "
//                               " events/cyle: " << double(presort_events)/double(presort_cycles) << ", "
//                               " compares/cycle: " << double(presort_compares)/double(presort_cycles);

      bool noempties = false;
      while (!noempties)
      {
        noempties = true;
        for (auto i = current_spills.begin(); i != current_spills.end(); i++)
          if ((*i)->events.empty())
          {
            out_spill->time = (*i)->time;
            out_spill->data = (*i)->data;
            out_spill->stats = (*i)->stats;
            out_spill->detectors = (*i)->detectors;
            out_spill->state = (*i)->state;
            spectra->add_spill(out_spill);

            delete (*i);
            current_spills.erase(i);

            delete out_spill;
            out_spill = new Spill;
            noempties = false;
            break;
          }
      }
      delete out_spill;
    }

    if ((in_spill == nullptr) && current_spills.empty())
      break;
  }

  DBG << "<Engine> Spectra builder terminating";

  spectra->flush();
}


}
