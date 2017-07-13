#include "spectrum_event_mode.h"
#include "custom_logger.h"

SpectrumEventMode::SpectrumEventMode()
  : Spectrum()
{
  Setting attributes = metadata_.attributes();

  SettingMeta totalcoinc("total_coinc", SettingType::precise);
  totalcoinc.set_val("min", 0);
  totalcoinc.set_val("description", "Total coincidence count");
  totalcoinc.set_flag("readonly");
  Setting totc(totalcoinc);
  totc.set_number(0);
  attributes.branches.add(totc);

  Setting det({"detector", SettingType::stem});
  det.set_indices({-1});

  SettingMeta ignore_zero("cutoff_logic", SettingType::integer);
  ignore_zero.set_flag("preset");
  ignore_zero.set_val("description", "Hits rejected below minimum energy (before coincidence logic)");
  ignore_zero.set_val("min", 0);
  det.branches.add(ignore_zero);

  SettingMeta delay("delay_ns", SettingType::floating);
  delay.set_flag("preset");
  delay.set_val("description", "Digital delay (before coincidence logic)");
  delay.set_val("units", "ns");
  delay.set_val("min", 0);
  det.branches.add(delay);

  Setting dets({"per_detector", SettingType::stem});
  dets.branches.add(det);
  attributes.branches.add(dets);

  SettingMeta coinc_window("coinc_window", SettingType::floating);
  coinc_window.set_flag("preset");
  coinc_window.set_val("description", "Coincidence window");
  coinc_window.set_val("units", "ns");
  coinc_window.set_val("min", 0);
  attributes.branches.add(coinc_window);

  SettingMeta pattern_coinc("pattern_coinc", SettingType::pattern);
  pattern_coinc.set_flag("preset");
  pattern_coinc.set_val("description", "Coincidence pattern");
  pattern_coinc.set_val("chans", 1);
  attributes.branches.add(pattern_coinc);

  SettingMeta pattern_anti("pattern_anti", SettingType::pattern);
  pattern_anti.set_flag("preset");
  pattern_anti.set_val("description", "Anti-coincidence pattern");
  pattern_anti.set_val("chans", 1);
  attributes.branches.add(pattern_anti);

  SettingMeta pattern_add("pattern_add", SettingType::pattern);
  pattern_add.set_flag("preset");
  pattern_add.set_val("description", "Add pattern");
  pattern_add.set_val("chans", 1);
  attributes.branches.add(pattern_add);

  metadata_.overwrite_all_attributes(attributes);
}

bool SpectrumEventMode::_initialize()
{
  Spectrum::_initialize();

  pattern_coinc_ = metadata_.get_attribute("pattern_coinc").pattern();
  pattern_anti_ = metadata_.get_attribute("pattern_anti").pattern();
  pattern_add_ = metadata_.get_attribute("pattern_add").pattern();
  coinc_window_ = metadata_.get_attribute("coinc_window").get_number();
  if (coinc_window_ < 0)
    coinc_window_ = 0;

  max_delay_ = 0;
  Setting perdet = metadata_.get_attribute("per_detector");
  cutoff_logic_.resize(perdet.branches.size());
  delay_ns_.resize(perdet.branches.size());
  for (auto &d : perdet.branches)
  {
    int idx = -1;
    if (d.indices().size())
      idx = *d.indices().begin();
    if (idx >= static_cast<int16_t>(cutoff_logic_.size()))
    {
      cutoff_logic_.resize(idx + 1);
      delay_ns_.resize(idx + 1);
    }
    if (idx >= 0)
    {
      cutoff_logic_[idx] = d.find({"cutoff_logic"}, Match::id).get_number();
      delay_ns_[idx]     = d.find({"delay_ns"}, Match::id).get_number();
      if (delay_ns_[idx] > max_delay_)
        max_delay_ = delay_ns_[idx];
    }
  }
  max_delay_ += coinc_window_;

  return false; //still too abstract
}

void SpectrumEventMode::_init_from_file(std::string name)
{
  metadata_.set_attribute(Setting::precise("total_coinc", total_coincidences_), false);
  metadata_.set_attribute(Setting("pattern_coinc", pattern_coinc_), false);
  metadata_.set_attribute(Setting("pattern_anti", pattern_anti_), false);
  metadata_.set_attribute(Setting("pattern_add", pattern_add_), false);

  Spectrum::_init_from_file(name);
}

bool SpectrumEventMode::channel_relevant(int16_t channel) const
{
  return ((channel >= 0) &&
          (pattern_coinc_.relevant(channel) ||
           pattern_anti_.relevant(channel) ||
           pattern_add_.relevant(channel)));
}

bool SpectrumEventMode::event_relevant(const Event& e) const
{
  return this->channel_relevant(e.channel());
}

void SpectrumEventMode::_push_event(const Event& new_event)
{
  if (!this->event_relevant(new_event))
    return;

  Event event = new_event;
  if (event.channel() < static_cast<int16_t>(delay_ns_.size()))
    event.delay_ns(delay_ns_[event.channel()]);

  bool appended = false;
  bool pileup = false;
  if (backlog.empty() || backlog.back().past_due(event))
    backlog.push_back(Coincidence(event, coinc_window_, max_delay_));
  else
  {
    for (auto &q : backlog)
    {
      if (q.in_window(event))
      {
        if (q.add_event(event))
        {
          if (appended)
            DBG << "<" << metadata_.get_attribute("name").get_text() << "> "
                << "event " << event.debug() << " coincident with more than one other event (counted >=2 times)";
          appended = true;
        }
        else
        {
          DBG << "<" << metadata_.get_attribute("name").get_text() << "> "
              << "pileup event " << event.debug() << " with " << q.debug();
          pileup = true;
        }
      }
      else if (q.past_due(event))
        break;
      else if (q.antecedent(event))
        DBG << "<" << metadata_.get_attribute("name").get_text() << "> "
            << "antecedent event " << event.debug() << ". Something wrong with presorter or daq_device?";
    }

    if (!appended && !pileup)
      backlog.push_back(Coincidence(event, coinc_window_, max_delay_));
  }

  Coincidence evt;
  while (!backlog.empty() && (evt = backlog.front()).past_due(event))
  {
    backlog.pop_front();
    if (validate_coincidence(evt))
    {
      recent_count_++;
      total_coincidences_++;
      this->add_coincidence(evt);
    }
  }

}


bool SpectrumEventMode::validate_coincidence(const Coincidence &newEvent) const
{
  return (
        validate(newEvent, pattern_coinc_)
        &&
        antivalidate(newEvent, pattern_anti_)
        );
}

void SpectrumEventMode::_push_stats(const Status& newBlock)
{
  if (!this->channel_relevant(newBlock.channel()))
    return;

  Spectrum::_push_stats(newBlock);

  if (newBlock.channel() >= static_cast<int16_t>(energy_idx_.size()))
    energy_idx_.resize(newBlock.channel() + 1, -1);
  if (newBlock.event_model().name_to_val.count("energy"))
    energy_idx_[newBlock.channel()] = newBlock.event_model().name_to_val.at("energy");

  metadata_.set_attribute(Setting::precise("total_coinc", total_coincidences_), false);
}


void SpectrumEventMode::_flush()
{
  Spectrum::_flush();
  metadata_.set_attribute(Setting::precise("total_coinc", total_coincidences_), false);
}



