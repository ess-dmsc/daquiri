#include "DummyDevice.h"

#include "custom_logger.h"

DummyDevice::DummyDevice()
{
  std::string mp {"DummyDevice/"};

  add_dummy_settings();

  SettingMeta root("DummyDevice", SettingType::stem);
  root.set_flag("producer");

  root.set_enum(1000, mp + "DummySettings");
  add_definition(root);

  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

DummyDevice::~DummyDevice()
{
  daq_stop();
  die();
}

void DummyDevice::read_settings_bulk(Setting &set) const
{
  set = enrich_and_toggle_presets(set);
  set.set(Setting::indicator("DummyDevice/DummySettings/Indicator", dummy_selection_));
}

void DummyDevice::write_settings_bulk(const Setting& settings)
{
  auto set = enrich_and_toggle_presets(settings);
  dummy_selection_ = set.find({"DummyDevice/DummySettings/Menu"}).selection();
}

void DummyDevice::boot()
{
  if (!(status_ & ProducerStatus::can_boot))
  {
    WARN << "<DummyDevice> Cannot boot DummyDevice. Failed flag check (can_boot == 0)";
    return;
  }

  INFO << "<DummyDevice> Booting";
  status_ = ProducerStatus::loaded | ProducerStatus::booted;
}

void DummyDevice::die()
{
  INFO << "<DummyDevice> Shutting down";
  status_ = ProducerStatus::loaded | ProducerStatus::can_boot;
}

void DummyDevice::add_dummy_settings()
{
  std::string r {"DummyDevice/DummySettings"};

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

  SettingMeta a17(r + "/text", SettingType::text);
  add_definition(a17);

  SettingMeta a18(r + "/Color", SettingType::color);
  add_definition(a18);

  SettingMeta a19(r + "/File", SettingType::file);
  a19.set_val("wildcards", "Bash file (*.sh)");
  add_definition(a19);

  SettingMeta a20(r + "/Directory", SettingType::dir);
  add_definition(a20);

  SettingMeta a21(r + "/Detector", SettingType::detector);
  add_definition(a21);


  SettingMeta a22(r + "/Command", SettingType::command);
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
  a24b4.set_val("name", "name of a24b4");
  add_definition(a24b4);
  SettingMeta a24b8(r + "/Binary/bit8", SettingType::integer);
  a24b8.set_val("bits", 8);
  a24b8.set_val("name", "name of a24b8");
  add_definition(a24b8);


  SettingMeta a25(r + "/Indicator", SettingType::indicator);
  a25.set_enum(0, r + "/Indicator/a");
  a25.set_enum(1, r + "/Indicator/b");
  a25.set_enum(2, r + "/Indicator/c");
  add_definition(a25);
  SettingMeta a26(r + "/Indicator/a", SettingType::text);
  a26.set_val("name", "A");
  a26.set_val("color", "#FF0000");
  add_definition(a26);
  SettingMeta a27(r + "/Indicator/b", SettingType::text);
  a27.set_val("name", "B");
  a27.set_val("color", "#00FF00");
  add_definition(a27);
  SettingMeta a28(r + "/Indicator/c", SettingType::text);
  a28.set_val("name", "C");
  a28.set_val("color", "#0000FF");
  add_definition(a28);

  SettingMeta root(r, SettingType::stem);
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
  add_definition(root);
}
