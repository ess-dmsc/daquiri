#include <producers/DummyDevice/DummyDevice.h>

#include <core/util/custom_logger.h>

namespace DAQuiri {

DummyDevice::DummyDevice()
{
  std::string r{plugin_name()};

  add_dummy_settings();

  SettingMeta root(r, SettingType::stem);
  root.set_flag("producer");

  root.set_enum(1000, r + "/DummySettings");
  add_definition(root);

  manifest_["Stream1"].event_model.add_value("val1", 1000);
  manifest_["Stream1"].event_model.add_value("val2", 2000);
  manifest_["Stream1"].stats.branches.add(SettingMeta("float_val", SettingType::floating));
  manifest_["Stream1"].stats.branches.add(SettingMeta("int_val", SettingType::integer));
  manifest_["Stream1"].stats.branches.add(SettingMeta("precise_val", SettingType::precise));

  manifest_["Stream2"].event_model.add_value("val_a", 500);
  manifest_["Stream2"].event_model.add_trace("trc_a", {2, 3, 4});

  manifest_["Stream3"].event_model.add_trace("trc1", {5000});
  manifest_["Stream3"].event_model.add_trace("trc2", {200, 300});

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

DummyDevice::~DummyDevice()
{
  daq_stop();
  die();
}

Setting DummyDevice::settings() const
{
  auto set = get_rich_setting(plugin_name());

  std::string r{plugin_name() + "/DummySettings/"};
  set.set(Setting::boolean(r + "Enabled", !read_only_));
  set.set(Setting::integer(r + "Menu", dummy_selection_));
  set.set(Setting::indicator(r + "Indicator", dummy_selection_));

  set.set(Setting::integer(r + "IntUnbounded", int_unbounded_));
  set.set(Setting::integer(r + "IntBounded", int_bounded_));
  set.set(Setting::integer(r + "IntLB", int_lower_bounded_));
  set.set(Setting::integer(r + "IntUB", int_upper_bounded_));

  set.set(Setting::floating(r + "FloatUnbounded", float_unbounded_));
  set.set(Setting::floating(r + "FloatBounded", float_bounded_));
  set.set(Setting::floating(r + "FloatLB", float_lower_bounded_));
  set.set(Setting::floating(r + "FloatUB", float_upper_bounded_));

  set.set(Setting::precise(r + "PreciseUnbounded", precise_unbounded_));
  set.set(Setting::precise(r + "PreciseBounded", precise_bounded_));
  set.set(Setting::precise(r + "PreciseLB", precise_lower_bounded_));
  set.set(Setting::precise(r + "PreciseUB", precise_upper_bounded_));

  set.set(Setting(r + "Time", time_));
  set.set(Setting(r + "Duration", duration_));
  set.set(Setting(r + "Pattern", pattern_));
  set.set(Setting::boolean(r + "Boolean", bool_));

  auto sb = set.find({r + "Binary"});
  sb.set_int(binary_);
  set.set(sb);

  set.set(Setting::text(r + "Text", text_));
  set.set(Setting::text(r + "Color", color_));
  set.set(Setting::text(r + "File", file_));
  set.set(Setting::text(r + "Directory", directory_));
  set.set(Setting::text(r + "Detector", detector_));
  set.set(Setting::text(r + "Gradient", gradient_));

  set.enable_if_flag(!read_only_, "");
  set.enable_if_flag(true, "master");

  return set;
}

void DummyDevice::settings(const Setting& settings)
{
  std::string r{plugin_name() + "/DummySettings/"};

  auto set = enrich_and_toggle_presets(settings);
  dummy_selection_ = set.find({r + "Menu"}).selection();
  read_only_ = !set.find({r + "Enabled"}).triggered();

  int_unbounded_ = set.find({r + "IntUnbounded"}).get_int();
  int_lower_bounded_ = set.find({r + "IntLB"}).get_int();
  int_upper_bounded_ = set.find({r + "IntUB"}).get_int();
  int_bounded_ = set.find({r + "IntBounded"}).get_int();

  float_unbounded_ = set.find({r + "FloatUnbounded"}).get_number();
  float_lower_bounded_ = set.find({r + "FloatLB"}).get_number();
  float_upper_bounded_ = set.find({r + "FloatUB"}).get_number();
  float_bounded_ = set.find({r + "FloatBounded"}).get_number();

  precise_unbounded_ = set.find({r + "PreciseUnbounded"}).precise();
  precise_lower_bounded_ = set.find({r + "PreciseLB"}).precise();
  precise_upper_bounded_ = set.find({r + "PreciseUB"}).precise();
  precise_bounded_ = set.find({r + "PreciseBounded"}).precise();

  time_ = set.find({r + "Time"}).time();
  duration_ = set.find({r + "Duration"}).duration();

  pattern_ = set.find({r + "Pattern"}).pattern();
  bool_ = set.find({r + "Boolean"}).get_bool();

  text_ = set.find({r + "Text"}).get_text();
  color_ = set.find({r + "Color"}).get_text();
  file_ = set.find({r + "File"}).get_text();
  directory_ = set.find({r + "Directory"}).get_text();
  detector_ = set.find({r + "Detector"}).get_text();
  gradient_ = set.find({r + "Gradient"}).get_text();

  binary_ = set.find({r + "Binary"}).get_int();
}

StreamManifest DummyDevice::stream_manifest() const
{
  return manifest_;
}

void DummyDevice::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN("<DummyDevice> Cannot boot DummyDevice. Failed flag check (can_boot == 0)");
    return;
  }

  INFO("<DummyDevice> Booting");
  status_ = ProducerStatus::loaded | ProducerStatus::booted;
}

void DummyDevice::die()
{
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void DummyDevice::add_dummy_settings()
{
  std::string r{plugin_name() + "/DummySettings"};

  SettingMeta a0(r + "/Enabled", SettingType::boolean);
  a0.set_flag("master");
  add_definition(a0);

  SettingMeta a1(r + "/IntUnbounded", SettingType::integer);
  add_definition(a1);

  SettingMeta a2(r + "/IntLB", SettingType::integer);
  a2.set_val("min", 0);
  add_definition(a2);

  SettingMeta a3(r + "/IntUB", SettingType::integer);
  a3.set_val("max", 0);
  add_definition(a3);

  SettingMeta a4(r + "/IntBounded", SettingType::integer);
  a4.set_val("min", 2);
  a4.set_val("max", 4);
  add_definition(a4);

  SettingMeta a5(r + "/FloatUnbounded", SettingType::floating);
  a5.set_val("step", 0.05);
  add_definition(a5);

  SettingMeta a6(r + "/FloatLB", SettingType::floating);
  a6.set_val("min", 0);
  a6.set_val("step", 0.2);
  add_definition(a6);

  SettingMeta a7(r + "/FloatUB", SettingType::floating);
  a7.set_val("max", 0);
  a7.set_val("step", 0.5);
  add_definition(a7);

  SettingMeta a8(r + "/FloatBounded", SettingType::floating);
  a8.set_val("min", 2);
  a8.set_val("max", 4);
  a8.set_val("step", 0.1);
  add_definition(a8);

  SettingMeta a9(r + "/PreciseUnbounded", SettingType::precise);
  a9.set_val("step", 0.05);
  add_definition(a9);

  SettingMeta a10(r + "/PreciseLB", SettingType::precise);
  a10.set_val("min", 0);
  a10.set_val("step", 0.2);
  add_definition(a10);

  SettingMeta a11(r + "/PreciseUB", SettingType::precise);
  a11.set_val("max", 0);
  a11.set_val("step", 0.5);
  add_definition(a11);

  SettingMeta a12(r + "/PreciseBounded", SettingType::precise);
  a12.set_val("min", 2);
  a12.set_val("max", 4);
  a12.set_val("step", 0.1);
  add_definition(a12);

  SettingMeta a13(r + "/Time", SettingType::time);
  add_definition(a13);

  SettingMeta a14(r + "/Duration", SettingType::duration);
  add_definition(a14);

  SettingMeta a15(r + "/Pattern", SettingType::pattern);
  a15.set_val("chans", 6);
  add_definition(a15);

  SettingMeta a16(r + "/Boolean", SettingType::boolean);
  add_definition(a16);

  SettingMeta a17(r + "/Text", SettingType::text);
  add_definition(a17);

  SettingMeta a18(r + "/Color", SettingType::text);
  a18.set_flag("color");
  add_definition(a18);

  SettingMeta a19(r + "/File", SettingType::text);
  a19.set_flag("file");
  a19.set_val("wildcards", "Bash file (*.sh)");
  add_definition(a19);

  SettingMeta a20(r + "/Directory", SettingType::text);
  a20.set_flag("directory");
  add_definition(a20);

  SettingMeta a21(r + "/Detector", SettingType::text);
  a21.set_flag("detector");
  add_definition(a21);

  SettingMeta a22(r + "/Command", SettingType::command);
  a22.set_val("command_name", "Do something");
  add_definition(a22);

  SettingMeta a23(r + "/Menu", SettingType::menu);
  a23.set_enum(0, "a");
  a23.set_enum(1, "b");
  a23.set_enum(2, "c");
  add_definition(a23);

  SettingMeta a24(r + "/Binary", SettingType::binary);
  a24.set_val("bits", 16);
  a24.set_enum(0, "bit0");
  a24.set_enum(1, "bit1");
  a24.set_enum(3, "bit3");
  a24.set_enum(4, r + "/Binary/bit4");
  a24.set_enum(8, r + "/Binary/bit8");
  add_definition(a24);
  SettingMeta a24b4(r + "/Binary/bit4", SettingType::menu);
  a24b4.set_enum(0, "c0");
  a24b4.set_enum(1, "c1");
  a24b4.set_enum(2, "c2");
  a24b4.set_enum(3, "c3");
  a24b4.set_val("name", "Bravo");
  add_definition(a24b4);
  SettingMeta a24b8(r + "/Binary/bit8", SettingType::integer);
  a24b8.set_val("bits", 8);
  a24b8.set_val("name", "Charlie");
  add_definition(a24b8);

  SettingMeta a25(r + "/Indicator", SettingType::indicator);
  a25.set_enum(0, r + "/Indicator/a");
  a25.set_enum(1, r + "/Indicator/b");
  a25.set_enum(2, r + "/Indicator/c");
  add_definition(a25);
  SettingMeta a26(r + "/Indicator/a", SettingType::text);
  a26.set_val("name", "A");
  a26.set_val("color", "#F00000");
  add_definition(a26);
  SettingMeta a27(r + "/Indicator/b", SettingType::text);
  a27.set_val("name", "B");
  a27.set_val("color", "#00F000");
  add_definition(a27);
  SettingMeta a28(r + "/Indicator/c", SettingType::text);
  a28.set_val("name", "C");
  a28.set_val("color", "#0000F0");
  add_definition(a28);

  SettingMeta a29(r + "/Gradient", SettingType::text);
  a29.set_flag("gradient-name");
  add_definition(a29);

  SettingMeta root(r, SettingType::stem);
  root.set_enum(0, a0.id());
  root.set_enum(1, a1.id());
  root.set_enum(2, a2.id());
  root.set_enum(3, a3.id());
  root.set_enum(4, a4.id());
  root.set_enum(5, a5.id());
  root.set_enum(6, a6.id());
  root.set_enum(7, a7.id());
  root.set_enum(8, a8.id());
  root.set_enum(9, a9.id());
  root.set_enum(10, a10.id());
  root.set_enum(11, a11.id());
  root.set_enum(12, a12.id());
  root.set_enum(13, a13.id());
  root.set_enum(14, a14.id());
  root.set_enum(15, a15.id());
  root.set_enum(16, a16.id());
  root.set_enum(17, a17.id());
  root.set_enum(18, a18.id());
  root.set_enum(19, a19.id());
  root.set_enum(20, a20.id());
  root.set_enum(21, a21.id());
  root.set_enum(22, a22.id());
  root.set_enum(23, a23.id());
  root.set_enum(24, a24.id());
  root.set_enum(25, a25.id());
  root.set_enum(29, a29.id());

  add_definition(root);
}

}