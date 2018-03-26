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

int main(int argc, char **argv)
{
  hdf5::error::Singleton::instance().auto_print(false);

  producers_autoreg();
  consumers_autoreg();

  int duration = 1;
  std::string durstr;
  if (argc > 1)
    durstr = std::string (argv[1]);
  if (is_number(durstr))
    duration = std::stoi(durstr);
  if (duration < 1)
    duration = 1;

  auto& engine = Engine::singleton();
  engine.initialize(get_profile());

  engine.set_setting(Setting::integer("MockProducer/ValueCount", 3), Match::id);
  define_value(engine, 0, "energy", 50, 2500);
  define_value(engine, 1, "x", 30, 2500);
  define_value(engine, 2, "y", 70, 2500);

  engine.boot();

  DBG << "\n" << engine.pull_settings().debug("   ", false);

  if (0 == (engine.status() & ProducerStatus::can_run))
  {
    DBG << "Engine cannot run";
    return 1;
  }

  ProjectPtr project = ProjectPtr( new Project() );

  project->set_prototypes(get_prototypes());

  Interruptor interruptor(false);
  engine.acquire(project, interruptor, duration);
  engine.die();

  DBG << "\n" << *project;

  DBG << "============================";
  DBG << "======testing file ops======";
  DBG << "============================";

  project->save_split("./test_split");
  project->save_as("./results.h5");
  ProjectPtr project2 = ProjectPtr( new Project() );
  project2->open("./results.h5");

  DBG << "\n" << *project2;

  return 0;
}

Setting get_profile()
{
  auto profile = Engine::default_settings();

  auto settings = ProducerFactory::singleton().default_settings("MockProducer");
  settings.set_text("producer1");
  settings.set(Setting::text("MockProducer/StreamID", "exy"));
  settings.set(Setting::floating("MockProducer/SpillInterval", 0.2));
  settings.set(Setting::integer("MockProducer/Resolution", 16));
  settings.set(Setting::floating("MockProducer/CountRate", 50000));
  settings.set(Setting::floating("MockProducer/DeadTime", 5));
  profile.branches.add(settings);

  return profile;
}

void define_value(Engine& e, uint16_t num,
                  std::string name, double center, double spread)
{
  auto n = Setting::text("MockProducer/Value/Name", name);
  auto c = Setting::floating("MockProducer/Value/PeakCenter", center);
  auto s = Setting::floating("MockProducer/Value/PeakSpread", spread);
  n.set_indices({num});
  c.set_indices({num});
  s.set_indices({num});
  e.set_setting(n, Match::id | Match::indices);
  e.set_setting(c, Match::id | Match::indices);
  e.set_setting(s, Match::id | Match::indices);
}


Container<ConsumerMetadata> get_prototypes()
{
  Container<ConsumerMetadata> prototypes;

  auto ptype = ConsumerFactory::singleton().create_prototype("Histogram 1D");
  ptype.set_attribute(Setting::integer("downsample", 9));
  ptype.set_attribute(Setting::text("stream_id", "exy"));
  ptype.set_attribute(Setting::text("value_name", "energy"));
  prototypes.add(ptype);

  auto itype = ConsumerFactory::singleton().create_prototype("Histogram 2D");
  itype.set_attribute(Setting::integer("downsample", 10));
  itype.set_attribute(Setting::text("stream_id", "exy"));
  itype.set_attribute(Setting::text("x_name", "x"));
  itype.set_attribute(Setting::text("y_name", "y"));
  prototypes.add(itype);

  auto vtype = ConsumerFactory::singleton().create_prototype("Histogram 3D");
  vtype.set_attribute(Setting::integer("downsample", 11));
  vtype.set_attribute(Setting::text("stream_id", "exy"));
  vtype.set_attribute(Setting::text("x_name", "x"));
  vtype.set_attribute(Setting::text("y_name", "y"));
  vtype.set_attribute(Setting::text("z_name", "energy"));
  prototypes.add(vtype);

  auto pbtype = ConsumerFactory::singleton().create_prototype("Prebinned 1D");
  pbtype.set_attribute(Setting::text("stream_id", "exy"));
  pbtype.set_attribute(Setting::text("value_name", "x"));
  prototypes.add(pbtype);

  auto tdtype = ConsumerFactory::singleton().create_prototype("Time-Activity 1D");
  tdtype.set_attribute(Setting::text("stream_id", "exy"));
  tdtype.set_attribute(Setting::floating("time_resolution", 25));
  tdtype.set_attribute(Setting::integer("time_units", 6));
  prototypes.add(tdtype);

  auto tstype = ConsumerFactory::singleton().create_prototype("TimeSpectrum 2D");
  tstype.set_attribute(Setting::text("stream_id", "exy"));
  tstype.set_attribute(Setting::text("value_name", "x"));
  tstype.set_attribute(Setting::integer("downsample", 10));
  tstype.set_attribute(Setting::floating("time_resolution", 25));
  tstype.set_attribute(Setting::integer("time_units", 6));
  prototypes.add(tstype);

  return prototypes;
}
