#include "engine.h"

#include "custom_logger.h"
#include "lexical_extensions.h"

#include "consumers_autoreg.h"
#include "producers_autoreg.h"

#include "producer_factory.h"
#include "consumer_factory.h"

#include "custom_timer.h"

#include "h5json.h"

using namespace DAQuiri;

Setting get_profile();
Container<ConsumerMetadata> get_prototypes();
void define_value(Engine& e, uint16_t num,
                  std::string name, double center, double spread);

int main(int argc, char** argv)
{
  hdf5::error::Singleton::instance().auto_print(false);

  producers_autoreg();
  consumers_autoreg();

  int duration = 1;
  std::string durstr;
  if (argc > 1)
    durstr = std::string(argv[1]);
  if (is_number(durstr))
    duration = std::stoi(durstr);
  if (duration < 1)
    duration = 1;

  auto& engine = Engine::singleton();
  engine.initialize(get_profile());

  engine.set_setting(Setting::integer("MockProducer/ValueCount", 4), Match::id);
  define_value(engine, 0, "x", 30, 2500);
  define_value(engine, 1, "y", 50, 2500);
  define_value(engine, 2, "z", 70, 2500);
  define_value(engine, 3, "energy", 1, 3000);

  engine.boot();

  DBG << "\n" << engine.settings().debug("   ", false);

  if (0 == (engine.status() & ProducerStatus::can_run))
  {
    DBG << "Engine cannot run";
    return 1;
  }

  ProjectPtr project = ProjectPtr(new Project());

  for (const auto& definition : get_prototypes())
    project->add_consumer(ConsumerFactory::singleton().create_from_prototype(definition));

  Interruptor interruptor(false);
  engine.acquire(project, interruptor, duration);
  engine.die();

  DBG << "============================";
  DBG << "======testing file ops======";
  DBG << "============================";

  project->save_split("./results_split");
  project->save("./results.h5");

  ProjectPtr project2 = ProjectPtr(new Project());
  project2->open("./results.h5");

  std::stringstream ss1, ss2;
  ss1 << *project;
  ss2 << *project2;

  DBG << "\n" << ss1.str();

//  std::ofstream ofs1("ss1.txt", std::ofstream::out | std::ofstream::trunc);
//  ofs1 << ss1.str();
//
//  std::ofstream ofs2("ss2.txt", std::ofstream::out | std::ofstream::trunc);
//  ofs2 << ss2.str();

  if (ss1.str() != ss2.str())
  {
    DBG << "Saved project not identical to original!";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

Setting get_profile()
{
  auto settings = ProducerFactory::singleton().default_settings("MockProducer");
  settings.set_text("producer1");
  settings.set(Setting::text("MockProducer/StreamID", "exy"));
  settings.set(Setting::floating("MockProducer/SpillInterval", 0.2));
  settings.set(Setting::floating("MockProducer/CountRate", 50000));
  settings.set(Setting::floating("MockProducer/DeadTime", 5));

  settings.set(Setting::integer("TimeBase/multiplier", 1));
  settings.set(Setting::integer("TimeBase/divider", 3));

  auto profile = Engine::default_settings();
  profile.branches.add(settings);
  return profile;
}

void define_value(Engine& e, uint16_t num,
                  std::string name, double center, double spread)
{
  auto n = Setting::text("Value/Name", name);
  auto c = Setting::floating("Value/PeakCenter", center);
  auto s = Setting::floating("Value/PeakSpread", spread);
  auto tl = Setting::integer("Value/TraceLength", 30);
  auto res = Setting::integer("Value/Resolution", 16);
  n.set_indices({num});
  c.set_indices({num});
  s.set_indices({num});
  tl.set_indices({num});
  res.set_indices({num});
  e.set_setting(n, Match::id | Match::indices);
  e.set_setting(c, Match::id | Match::indices);
  e.set_setting(s, Match::id | Match::indices);
  e.set_setting(tl, Match::id | Match::indices);
  e.set_setting(res, Match::id | Match::indices);
}

Container<ConsumerMetadata> get_prototypes()
{
  Container<ConsumerMetadata> prototypes;

  auto ptype = ConsumerFactory::singleton().create_prototype("Histogram 1D");
  ptype.set_attribute(Setting::integer("downsample", 10));
  ptype.set_attribute(Setting::text("stream_id", "exy"));
  ptype.set_attribute(Setting::text("value_name", "x"));
  prototypes.add(ptype);

  auto pbtype = ConsumerFactory::singleton().create_prototype("Prebinned 1D");
  pbtype.set_attribute(Setting::text("stream_id", "exy"));
  pbtype.set_attribute(Setting::text("value_name", "x"));
  prototypes.add(pbtype);

  auto toftype = ConsumerFactory::singleton().create_prototype("Time of Flight 1D");
  toftype.set_attribute(Setting::text("stream_id", "exy"));
  toftype.set_attribute(Setting::floating("time_resolution", 10));
  toftype.set_attribute(Setting::integer("time_units", 6));
  prototypes.add(toftype);

  auto tdtype = ConsumerFactory::singleton().create_prototype("Time-Activity 1D");
  tdtype.set_attribute(Setting::text("stream_id", "exy"));
  tdtype.set_attribute(Setting::floating("time_resolution", 25));
  tdtype.set_attribute(Setting::integer("time_units", 6));
  prototypes.add(tdtype);

  auto itype = ConsumerFactory::singleton().create_prototype("Histogram 2D");
  itype.set_attribute(Setting::integer("downsample", 11));
  itype.set_attribute(Setting::text("stream_id", "exy"));
  itype.set_attribute(Setting::text("x_name", "x"));
  itype.set_attribute(Setting::text("y_name", "y"));
  prototypes.add(itype);

  auto imtype = ConsumerFactory::singleton().create_prototype("Image 2D");
  imtype.set_attribute(Setting::integer("downsample", 11));
  imtype.set_attribute(Setting::text("stream_id", "exy"));
  imtype.set_attribute(Setting::text("x_name", "x"));
  imtype.set_attribute(Setting::text("y_name", "y"));
  imtype.set_attribute(Setting::text("val_name", "energy"));
  prototypes.add(imtype);

  auto tof2type = ConsumerFactory::singleton().create_prototype("Time of Flight 2D");
  tof2type.set_attribute(Setting::text("stream_id", "exy"));
  tof2type.set_attribute(Setting::integer("downsample", 10));
  tof2type.set_attribute(Setting::text("value_name", "x"));
  tof2type.set_attribute(Setting::floating("time_resolution", 10));
  tof2type.set_attribute(Setting::integer("time_units", 6));
  prototypes.add(tof2type);

  auto tstype = ConsumerFactory::singleton().create_prototype("TimeSpectrum 2D");
  tstype.set_attribute(Setting::text("stream_id", "exy"));
  tstype.set_attribute(Setting::text("value_name", "x"));
  tstype.set_attribute(Setting::integer("downsample", 11));
  tstype.set_attribute(Setting::floating("time_resolution", 25));
  tstype.set_attribute(Setting::integer("time_units", 6));
  prototypes.add(tstype);

  auto vtype = ConsumerFactory::singleton().create_prototype("Histogram 3D");
  vtype.set_attribute(Setting::integer("downsample", 12));
  vtype.set_attribute(Setting::text("stream_id", "exy"));
  vtype.set_attribute(Setting::text("x_name", "x"));
  vtype.set_attribute(Setting::text("y_name", "y"));
  vtype.set_attribute(Setting::text("z_name", "z"));
  prototypes.add(vtype);

  return prototypes;
}
