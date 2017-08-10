#include "consumer.h"
#include "h5json.h"
#include "ascii_tree.h"

#include "custom_timer.h"
#include "custom_logger.h"

#define SLEEP_TIME_MS 200

namespace DAQuiri {

DataAxis::DataAxis(Calibration c, size_t resolution)
{
  calibration = c;
  domain.resize(resolution);
  for (size_t i=0; i < resolution; ++i)
    domain[i] = calibration.transform(i);
}

DataAxis::DataAxis(Calibration c, size_t resolution, uint16_t bits)
{
  calibration = c;
  domain.resize(resolution);
  for (size_t i=0; i < resolution; ++i)
    domain[i] = calibration.transform(i, bits);
}

Pair DataAxis::bounds() const
{
  return Pair(0, domain.size());
}

std::string DataAxis::label() const
{
//  if (!calibration.valid())
//    return "undefined axis";
  std::stringstream ss;
  if (!calibration.to().value.empty())
    ss << calibration.to().value;
  else
    ss << calibration.from().value;
  if (!calibration.to().units.empty())
    ss << " (" << calibration.to().units << ")";
  else
    ss << " (" << calibration.from().bits << " bits)";
  return ss.str();
}

std::string DataAxis::debug() const
{
  std::stringstream ss;
  ss << "domain_size=" << domain.size();
  if (domain.size())
    ss << " [" << domain[0]
       << "-" << domain[domain.size()-1]
       << "]";
  if (calibration.valid())
    ss << " " << calibration.debug();
  return ss.str();
}


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

PreciseFloat Consumer::data(std::initializer_list<size_t> list ) const
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  if (list.size() != this->metadata_.dimensions())
    return 0;
  return this->_data(list);
}

std::unique_ptr<EntryList> Consumer::data_range(std::initializer_list<Pair> list)
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  return this->_data_range(list);
}

void Consumer::append(const Entry& e)
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  if (metadata_.dimensions() < 1)
    return;
  else
    this->_append(e);
}

bool Consumer::from_prototype(const ConsumerMetadata& newtemplate)
{
  boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
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
  boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
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
  boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  this->_flush();
}

DataAxis Consumer::axis(uint16_t dimension) const
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  
  if (dimension < axes_.size())
    return axes_[dimension];
  else
    return DataAxis();
}

bool Consumer::changed() const
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  return changed_;
}

void Consumer::set_detectors(const std::vector<Detector>& dets)
{
  boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  
  this->_set_detectors(dets);
  changed_ = true;
}

void Consumer::reset_changed()
{
  boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  changed_ = false;
}

//accessors for various properties
ConsumerMetadata Consumer::metadata() const
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  return metadata_;
}

std::string Consumer::type() const
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  return my_type();
}

uint16_t Consumer::dimensions() const
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  return metadata_.dimensions();
}

std::string Consumer::debug(std::string prepend, bool verbose) const
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);
  std::stringstream ss;
  ss << prepend << my_type();
  if (changed_)
    ss << "(changed)";
  ss << "\n";
  if (axes_.empty())
    ss << prepend << k_branch_mid_B << "Axes undefined\n";
  else
  {
    ss << prepend << k_branch_mid_B << "Axes:\n";
    for (size_t i=0; i < axes_.size();++i)
    {
      ss << prepend << k_branch_pre_B
         << (((i+1) == axes_.size()) ? k_branch_end_B : k_branch_mid_B)
         << i << "   " << axes_.at(i).debug()
         << "\n";
    }
  }
  ss << prepend << k_branch_mid_B
     << metadata_.debug(prepend + k_branch_pre_B, verbose);
  ss << prepend << k_branch_end_B << "Data:\n"
     << this->_data_debug(prepend + "  ");
  return ss.str();
}


//change stuff

void Consumer::set_attribute(const Setting &setting, bool greedy)
{
  boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);
  metadata_.set_attribute(setting, greedy);
  changed_ = true;
}

void Consumer::set_attributes(const Setting &settings)
{
  boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
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
  boost::unique_lock<boost::mutex> uniqueLock(unique_mutex_, boost::defer_lock);
  while (!uniqueLock.try_lock())
    wait_ms(SLEEP_TIME_MS);

  if (!g.has_group("metadata"))
    return false;

  from_json(json(g.open_group("metadata")), metadata_);
//  metadata_.from_json(g.open_group("metadata"));

  bool ret = this->_initialize();

  if (ret && withdata)
    this->_load_data(g);

  if (ret)
    this->_recalc_axes();

  return ret;
}

bool Consumer::save(H5CC::Group& g) const
{
  boost::shared_lock<boost::shared_mutex> lock(shared_mutex_);

  g.write_attribute("type", this->my_type());

//  json j = metadata_.to_json();
  auto mdg = g.require_group("metadata");

  H5CC::from_json(json(metadata_), mdg);

  this->_save_data(g);
}

}
