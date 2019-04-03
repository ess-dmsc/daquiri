#include <producers/DetectorIndex/DetectorIndex.h>

#include <core/util/logger.h>

DetectorIndex::DetectorIndex()
{
  define_settings();
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

DetectorIndex::~DetectorIndex()
{
  daq_stop();
  die();
}

void DetectorIndex::define_settings()
{
  std::string r{plugin_name()};

  SettingMeta det_count(r + "/DetectorCount", SettingType::integer, "Detector count");
  det_count.set_val("min", 1);
  add_definition(det_count);

  SettingMeta det(r + "/Detector", SettingType::text, "Detector");
  det.set_flag("detector");
  add_definition(det);

  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  add_definition(root);
}

Setting DetectorIndex::settings() const
{
  std::string r{plugin_name()};

  auto set = get_rich_setting(r);

  Setting total_det_num = get_rich_setting(r + "/DetectorCount");
  total_det_num.set_number(detectors_.size());
  set.branches.add_a(total_det_num);

  SettingMeta m = setting_definitions_.at(r + "/Detector");
  for (size_t i = 0; i < detectors_.size(); ++i)
  {
    m.set_val("name", "Detector " + std::to_string(i));
    Setting det(m);
    det.set_text(detectors_[i].id());
    det.set_indices({int32_t(i)});
    set.branches.add_a(det);
  }

  set.enable_if_flag(!(status_ & booted), "preset");

  return set;
}

void DetectorIndex::settings(const Setting& settings)
{
  auto set = enrich_and_toggle_presets(settings);

  std::string r{plugin_name()};

  Setting totaldets = set.find(Setting::integer(r + "/DetectorCount", 0));
  int oldtotal = detectors_.size();
  int newtotal = totaldets.get_number();
  if (newtotal < 0)
    newtotal = 0;

  if (oldtotal != newtotal)
    detectors_.resize(newtotal);

  for (auto b : set.branches)
  {
    if (b.id() != (r + "/Detector"))
      continue;

    auto indices = b.indices();
    if (indices.size() != 1)
      continue;
    auto idx = *indices.begin();
    if (idx < 0 || idx >= detectors_.size())
      continue;

    //hackity hack
    detectors_[idx].set_id(b.get_text());
  }
}

void DetectorIndex::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN("<DetectorIndex> Cannot boot DetectorIndex. Failed flag check (!can_boot)");
    return;
  }
  status_ = ProducerStatus::loaded | ProducerStatus::booted;
}

void DetectorIndex::die()
{
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

