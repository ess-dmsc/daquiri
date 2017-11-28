#include "consumer.h"
#include "h5json.h"
#include "ascii_tree.h"

#include "custom_logger.h"
#include "custom_timer.h"

namespace DAQuiri {

Consumer::Consumer()
{
  Setting attributes = metadata_.attributes();

  SettingMeta stream("stream_id", SettingType::text, "Stream ID");
  stream.set_flag("preset");
  attributes.branches.add(stream);

  metadata_.overwrite_all_attributes(attributes);
}

bool Consumer::_initialize()
{
  metadata_.disable_presets();
  stream_id_ = metadata_.get_attribute("stream_id").get_text();

  return false; //abstract sink indicates failure to init
}

void Consumer::_init_from_file()
{
  this->_initialize();
  this->_recalc_axes();
  this->_flush();
}

bool Consumer::from_prototype(const ConsumerMetadata& newtemplate)
{
  UNIQUE_LOCK_EVENTUALLY_ST

  if (metadata_.type() != newtemplate.type())
    return false;

  for (const auto& a : newtemplate.attributes_flat())
  {
    if (a.metadata().has_flag("readonly") && !a.metadata().has_flag("preset"))
      continue;
    metadata_.set_attribute(a);
  }
//  metadata_.set_attributes(newtemplate.attributes_flat());

  metadata_.detectors.clear(); // really?

  return (this->_initialize());
//  DBG << "<Consumer::from_prototype>" << metadata_.get_attribute("name").value_text << " made with dims=" << metadata_.dimensions();
//  DBG << "from prototype " << metadata_.debug();
//  mutex_.unlock();
}

void Consumer::push_spill(const Spill& spill)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  this->_push_spill(spill);
}

bool Consumer::_accept_spill(const Spill& spill)
{
  return (spill.stream_id == stream_id_);
}

void Consumer::_push_spill(const Spill& spill)
{
//  CustomTimer addspill_timer(true);

//  if (!spill.detectors.empty())
//    this->_set_detectors(spill.detectors);

  this->_push_stats_pre(spill);

  if (this->_accept_events())
    for (auto &q : spill.events)
      this->_push_event(q);

  this->_push_stats_post(spill);

//  DBG << "<" << metadata_.get_attribute("name").get_text() << "> added "
//      << spill.events.size() << " events in "
//      << addspill_timer.ms() << " ms at "
//      << addspill_timer.us() / double(spill.events.size()) << " us/hit";
}

void Consumer::flush()
{
  UNIQUE_LOCK_EVENTUALLY_ST
  this->_flush();
}

bool Consumer::changed() const
{
  SHARED_LOCK_ST
  return changed_;
}

void Consumer::set_detectors(const std::vector<Detector>& dets)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  this->_set_detectors(dets);
  changed_ = true;
}

void Consumer::reset_changed()
{
  UNIQUE_LOCK_EVENTUALLY_ST
  changed_ = false;
}

//accessors for various properties
ConsumerMetadata Consumer::metadata() const
{
  SHARED_LOCK_ST
  return metadata_;
}

DataspacePtr Consumer::data() const
{
  SHARED_LOCK_ST
  return DataspacePtr(data_->clone());
}

std::string Consumer::type() const
{
  SHARED_LOCK_ST
  return my_type();
}

uint16_t Consumer::dimensions() const
{
  SHARED_LOCK_ST
  if (data_)
    return data_->dimensions();
  return 0;
}

std::string Consumer::debug(std::string prepend, bool verbose) const
{
  SHARED_LOCK_ST
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
  UNIQUE_LOCK_EVENTUALLY_ST
  metadata_.set_attribute(setting, greedy);
  changed_ = true;
}

void Consumer::set_attributes(const Setting &settings)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  metadata_.set_attributes(settings.branches.data(), true);
  changed_ = true;
}




/////////////////////
/// Save and load ///
/////////////////////

bool Consumer::load(hdf5::node::Group& g, bool withdata)
{
  UNIQUE_LOCK_EVENTUALLY_ST
  if (!hdf5::has_group(g, "metadata"))
    return false;

  json j;
  hdf5::to_json(j, hdf5::node::Group(g["metadata"]));
  metadata_ = j;
//  metadata_.from_json(g.open_group("metadata"));

  bool ret = this->_initialize();

  if (ret && withdata && data_)
    data_->load(g);

  if (ret)
    this->_recalc_axes();

  return ret;
}

void Consumer::save(hdf5::node::Group& g) const
{
  SHARED_LOCK_ST
  hdf5::attribute::Attribute a = g.attributes.create<std::string>("type");
  a.write(this->my_type());
//  g.write_attribute("type", this->my_type());

  auto mdg = hdf5::require_group(g, "metadata");

  hdf5::from_json(json(metadata_), mdg);

  if (data_)
    data_->save(g);
}

}
