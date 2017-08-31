#include "engine.h"

#include "custom_logger.h"
#include "lexical_extensions.h"

#include "consumers_autoreg.h"
#include "producers_autoreg.h"

#include "producer_factory.h"

#include "custom_timer.h"

Setting get_profile();
Container<ConsumerMetadata> get_prototypes();
void define_value(Engine& e, uint16_t num,
                  std::string name, double center, double spread);

int main(int argc, char **argv)
{
  producers_autoreg();

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
  engine.boot();

  engine.set_setting(Setting::integer("MockProducer/ValueCount", 3), Match::id);
  define_value(engine, 0, "energy", 50, 2500);
  define_value(engine, 1, "x", 30, 2500);
  define_value(engine, 2, "y", 70, 2500);

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

  for (auto s : project->get_sinks())
    DBG << "Result[" << s.first << "]\n"
        << s.second->debug("", false);

  wait_ms(2000);

  return 0;
}

Setting get_profile()
{
  auto profile = Engine::singleton().pull_settings();
  profile.set(Setting::text("Profile description",
                            "Test profile for Mock Producer"));

  auto settings = ProducerFactory::singleton().default_settings("MockProducer");
  settings.set(Setting::integer("MockProducer/SpillInterval", 1));
  settings.set(Setting::integer("MockProducer/Resolution", 16));
  settings.set(Setting::floating("MockProducer/CountRate", 500000));
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

  ConsumerMetadata ptype = ConsumerFactory::singleton().create_prototype("Histogram 1D");
  ptype.set_attribute(Setting::text("name", "Spectrum"));
  ptype.set_attribute(Setting::integer("resolution", 7));
  ptype.set_attribute(Setting::text("value_name", "energy"));
  ptype.set_attribute(Setting("channels", Pattern(1, {true})));

  ConsumerMetadata itype = ConsumerFactory::singleton().create_prototype("Image 2D");
  itype.set_attribute(Setting::text("name", "Image"));
  itype.set_attribute(Setting::integer("downsample", 10));
  itype.set_attribute(Setting::text("x_name", "x"));
  itype.set_attribute(Setting::text("y_name", "y"));
  itype.set_attribute(Setting("pattern_add", Pattern(1, {true})));

  prototypes.add(ptype);
  prototypes.add(itype);

  return prototypes;
}
