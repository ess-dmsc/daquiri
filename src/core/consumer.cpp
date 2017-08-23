#include "consumer.h"
#include "h5json.h"
#include "ascii_tree.h"

#include "custom_timer.h"
#include "custom_logger.h"

#define SLEEP_TIME_MS 200

using shared_lock = boost::shared_lock<boost::shared_mutex>;
using unique_lock = boost::unique_lock<boost::shared_mutex>;

namespace DAQuiri {

Consumer::Consumer()
{
  Setting attributes = metadata_.attributes();

  Setting name(SettingMeta("name", SettingType::text));
  attributes.branches.add(name);

  SettingMeta vis("visible", SettingType::boolean);
  vis.set_val("description", "Plot visible");
  attributes.branches.add(Setting(vis));

  SettingMeta rescale("rescale", SettingType::precise);
  rescale.set_val("description", "Rescale factor");
  rescale.set_val("min", 0);
  Setting resc(rescale);
  resc.set_number(1);
  attributes.branches.add(resc);

  SettingMeta descr("description", SettingType::text);
  descr.set_val("description", "Description");
  attributes.branches.add(Setting(descr));

  SettingMeta start_time("start_time", SettingType::time);
  start_time.set_val("description", "Start time");
  start_time.set_flag("readonly");
  attributes.branches.add(start_time);

  metadata_.overwrite_all_attributes(attributes);
}

bool Consumer::_initialize()
{
  metadata_.disable_presets();
  return false; //abstract sink indicates failure to init
}

void Consumer::_init_from_file(std::string name)
{
  metadata_.set_attribute(Setting::text("name", name), false);
  this->_initialize();
  this->_recalc_axes();
  this->_flush();
}

bool Consumer::from_prototype(const ConsumerMetadata& newtemplate)
{
  unique_lock uniqueLock(mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);

  if (metadata_.type() != newtemplate.type())
    return false;

  metadata_.overwrite_all_attributes(newtemplate.attributes());
  metadata_.detectors.clear(); // really?

  return (this->_initialize());
//  DBG << "<Consumer::from_prototype>" << metadata_.get_attribute("name").value_text << " made with dims=" << metadata_.dimensions();
//  DBG << "from prototype " << metadata_.debug();
}

void Consumer::push_spill(const Spill& spill)
{
  unique_lock uniqueLock(mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  this->_push_spill(spill);
}

void Consumer::_push_spill(const Spill& spill)
{
  //  CustomTimer addspill_timer(true);

  if (!spill.detectors.empty())
    this->_set_detectors(spill.detectors);

  for (auto &q : spill.events)
    this->_push_event(q);

  for (auto &q : spill.stats)
    this->_push_stats(q.second);

  //  addspill_timer.stop();
  //  DBG << "<" << metadata_.name << "> added " << events << " events in "
  //         << addspill_timer.ms() << " ms at " << addspill_timer.us() / events << " us/hit";

  //  DBG << "<" << metadata_.name << "> left in backlog " << backlog.size();
}

void Consumer::flush()
{
  unique_lock uniqueLock(mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  this->_flush();
}

bool Consumer::changed() const
{
  shared_lock lock(mutex_);
  return changed_;
}

void Consumer::set_detectors(const std::vector<Detector>& dets)
{
  unique_lock uniqueLock(mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  
  this->_set_detectors(dets);
  changed_ = true;
}

void Consumer::reset_changed()
{
  unique_lock uniqueLock(mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  changed_ = false;
}

//accessors for various properties
ConsumerMetadata Consumer::metadata() const
{
  shared_lock lock(mutex_);
  return metadata_;
}

DataspacePtr Consumer::data() const
{
  shared_lock lock(mutex_);
  return data_;
}

std::string Consumer::type() const
{
  shared_lock lock(mutex_);
  return my_type();
}

uint16_t Consumer::dimensions() const
{
  shared_lock lock(mutex_);
  if (data_)
    return data_->dimensions();
  return 0;
}

std::string Consumer::debug(std::string prepend, bool verbose) const
{
  shared_lock lock(mutex_);
  std::stringstream ss;
  ss << prepend << my_type();
  if (changed_)
    ss << "(changed)";
  ss << "\n";
  ss << prepend << k_branch_mid_B
     << metadata_.debug(prepend + k_branch_pre_B, verbose);
  if (data_)
    ss << data_->debug(prepend + k_branch_end_B);
  else
    ss << prepend << k_branch_end_B << "NODATA";
  return ss.str();
}

//change stuff

void Consumer::set_attribute(const Setting &setting, bool greedy)
{
  unique_lock uniqueLock(mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  metadata_.set_attribute(setting, greedy);
  changed_ = true;
}

void Consumer::set_attributes(const Setting &settings)
{
  unique_lock uniqueLock(mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  metadata_.set_attributes(settings.branches.data(), true);
  changed_ = true;
}




/////////////////////
//Save and load//////
/////////////////////

bool Consumer::load(H5CC::Group& g, bool withdata)
{
  unique_lock lock(mutex_, boost::defer_lock);
  while (!lock.try_lock())
    wait_ms(SLEEP_TIME_MS);

  if (!g.has_group("metadata"))
    return false;

  from_json(json(g.open_group("metadata")), metadata_);
//  metadata_.from_json(g.open_group("metadata"));

  bool ret = this->_initialize();

  if (ret && withdata && data_)
    data_->load(g);

  if (ret)
    this->_recalc_axes();

  return ret;
}

bool Consumer::save(H5CC::Group& g) const
{
  shared_lock lock(mutex_);

  g.write_attribute("type", this->my_type());

//  json j = metadata_.to_json();
  auto mdg = g.require_group("metadata");

  H5CC::from_json(json(metadata_), mdg);

  if (data_)
    data_->save(g);
}

}
