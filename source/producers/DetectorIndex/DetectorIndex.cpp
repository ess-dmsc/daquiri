#include "DetectorIndex.h"

#include "custom_logger.h"

DetectorIndex::DetectorIndex()
{
  add_setting_defs();

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

DetectorIndex::~DetectorIndex()
{
  daq_stop();
  die();
}

void DetectorIndex::read_settings_bulk(Setting &set) const
{
  set = enrich_and_toggle_presets(set);
  std::string r {"Detectors"};

  Setting total_det_num = get_rich_setting(r + "/DetectorCount");
  total_det_num.set_number(detectors_.size());

  set.branches.clear();
  set.branches.replace(total_det_num);

  SettingMeta m = setting_definitions_.at(r + "/Detector");
  for (size_t i=0; i < detectors_.size(); ++i)
  {
    m.set_val("name", "Detector " + std::to_string(i));
    Setting det(m);
    det.set_text(detectors_[i].id());
    det.set_indices({int32_t(i)});
    set.branches.add_a(det);
  }
}

void DetectorIndex::write_settings_bulk(const Setting& settings)
{
  auto set = enrich_and_toggle_presets(settings);

  std::string r {"Detectors"};

  Setting totaldets = set.find(Setting::integer(r + "/DetectorCount", 0));
  int oldtotal = detectors_.size();
  int newtotal = totaldets.get_number();
  if (newtotal < 0)
    newtotal = 0;

  if (oldtotal != newtotal)
    detectors_.resize(newtotal);

  for (auto b : set.branches)
  {
    if (b.id() != "Detector")
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
    WARN << "<DetectorIndex> Cannot boot DetectorIndex. Failed flag check (!can_boot)";
    return;
  }

  INFO << "<DetectorIndex> Booting";
  status_ = ProducerStatus::loaded | ProducerStatus::booted;
}

void DetectorIndex::die()
{
  INFO << "<DetectorIndex> Shutting down";
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void DetectorIndex::add_setting_defs()
{
  std::string r {"Detectors"};

  SettingMeta det_count(r + "/DetectorCount", SettingType::integer, "Detector count");
  det_count.set_val("min", 1);
  add_definition(det_count);

  SettingMeta det(r + "/Detector", SettingType::text, "Detector");
  det.set_flag("detector");
  add_definition(det);

  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");
  root.set_enum(0, det_count.id());
  root.set_enum(1, det.id());
  add_definition(root);
}
