#include <core/consumer.h>
#include <core/util/ascii_tree.h>
#include <core/util/h5json.h>
#include <core/util/logger.h>
#include <core/util/timer.h>

namespace DAQuiri {

Consumer::Consumer() {
  Setting attributes = metadata_.attributes();

  SettingMeta stream("stream_id", SettingType::text, "Stream ID");
  stream.set_flag("preset");
  stream.set_flag("stream");
  attributes.branches.add(stream);

  metadata_.overwrite_all_attributes(attributes);
}

void Consumer::_apply_attributes() {
  //  metadata_.disable_presets();
  stream_id_ = metadata_.get_attribute("stream_id").get_text();
}

void Consumer::_init_from_file() {
  this->_apply_attributes();
  this->_recalc_axes();
  this->_flush();
}

void Consumer::_set_detectors(const std::vector<Detector> &dets) {
  if (!data_)
    return;
  auto dims = data_->dimensions();

  metadata_.detectors.resize(dims, Detector());

  if (dets.size() == dims)
    metadata_.detectors = dets;

  if (dets.size() >= dims) {
    for (size_t i = 0; i < dets.size(); ++i) {
      if (metadata_.chan_relevant(i)) {
        metadata_.detectors[0] = dets[i];
        break;
      }
    }
  }

  this->_recalc_axes();
}

void Consumer::from_prototype(const ConsumerMetadata &newtemplate) {
  std::lock_guard<std::mutex> Guard(mutex_);

  if (metadata_.type() != newtemplate.type())
    return;

  for (const auto &a : newtemplate.attributes_flat()) {
    if (a.metadata().has_flag("readonly") && !a.metadata().has_flag("preset"))
      continue;
    metadata_.set_attribute(a);
    this->_apply_attributes();
  }

  metadata_.detectors.clear(); // really?
}

void Consumer::push_spill(const Spill &spill) {
  std::lock_guard<std::mutex> Guard(mutex_);
  this->_push_spill(spill);
}

bool Consumer::_accept_spill(const Spill &spill) {
  return (spill.stream_id == stream_id_);
}

void Consumer::_push_spill(const Spill &spill) {
  //  Timer addspill_timer(true);

  //  if (!spill.detectors.empty())
  //    this->_set_detectors(spill.detectors);

  this->_push_stats_pre(spill);

  if (this->_accept_spill(spill) && this->_accept_events(spill))
    for (auto &q : spill.events)
      this->_push_event(q);

  this->_push_stats_post(spill);

  //  DBG( "<" << metadata_.get_attribute("name").get_text() << "> added "
  //      << spill.events.size() << " events in "
  //      << addspill_timer.ms() << " ms at "
  //      << addspill_timer.us() / double(spill.events.size()) << " us/hit";
}

void Consumer::flush() {
  std::lock_guard<std::mutex> Guard(mutex_);
  this->_flush();
}

bool Consumer::changed() {
  std::lock_guard<std::mutex> Guard(mutex_);
  return changed_;
}

void Consumer::set_detectors(const std::vector<Detector> &dets) {
  std::lock_guard<std::mutex> Guard(mutex_);
  this->_set_detectors(dets);
  changed_ = true;
}

void Consumer::reset_changed() {
  std::lock_guard<std::mutex> Guard(mutex_);
  changed_ = false;
}

// accessors for various properties
ConsumerMetadata Consumer::metadata() {
  std::lock_guard<std::mutex> Guard(mutex_);
  return metadata_;
}

DataspacePtr Consumer::data() {
  std::lock_guard<std::mutex> Guard(mutex_);
  if (!data_)
    return nullptr;
  return DataspacePtr(data_->clone());
}

std::string Consumer::type() {
  std::lock_guard<std::mutex> Guard(mutex_);
  return my_type();
}

uint16_t Consumer::dimensions() {
  std::lock_guard<std::mutex> Guard(mutex_);
  if (data_)
    return data_->dimensions();
  return 0;
}

std::string Consumer::debug(std::string prepend, bool verbose) {
  std::lock_guard<std::mutex> Guard(mutex_);
  std::stringstream ss;
  ss << "COMSUMER";
  if (changed_)
    ss << " (changed)";
  ss << "\n";
  ss << prepend << k_branch_mid_B
     << metadata_.debug(prepend + k_branch_pre_B, verbose);
  if (data_)
    ss << prepend << k_branch_end_B << data_->debug(prepend + "  ");
  else
    ss << prepend << k_branch_end_B << "NODATA";
  return ss.str();
}

// change stuff

void Consumer::set_attribute(const Setting &setting, bool greedy) {
  std::lock_guard<std::mutex> Guard(mutex_);
  metadata_.set_attribute(setting, greedy);
  this->_apply_attributes();
  changed_ = true;
}

void Consumer::set_attributes(const Setting &settings) {
  std::lock_guard<std::mutex> Guard(mutex_);
  metadata_.set_attributes(settings.branches.data(), true);
  this->_apply_attributes();
  changed_ = true;
}

/////////////////////
/// Save and load ///
/////////////////////
void Consumer::load(hdf5::node::Group &g, bool withdata) {
  std::lock_guard<std::mutex> Guard(mutex_);
  if (!g.has_group("metadata"))
    return;

  try {
    json j;
    hdf5::to_json(j, hdf5::node::Group(g["metadata"]));
    metadata_ = j;

    this->_apply_attributes();

    if (withdata && data_)
      data_->load(g);

    this->_init_from_file();
  } catch (...) {
    std::throw_with_nested(
        std::runtime_error("<Consumer> Could not load " + metadata_.debug("")));
  }
}

void Consumer::save(hdf5::node::Group &g) {
  std::lock_guard<std::mutex> Guard(mutex_);
  try {
    hdf5::attribute::Attribute a = g.attributes.create<std::string>("type");
    a.write(this->my_type());

    auto mdg = hdf5::require_group(g, "metadata");
    hdf5::from_json(json(metadata_), mdg);

    if (data_)
      data_->save(g);
  } catch (...) {
    std::throw_with_nested(
        std::runtime_error("<Consumer> Could not save " + metadata_.debug("")));
  }
}

} // namespace DAQuiri
