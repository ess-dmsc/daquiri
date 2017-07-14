#include "engine.h"

#include "mock_producer.h"
#include "producer_factory.h"

#include "spectrum_events_1D.h"
#include "consumer_factory.h"

#include "custom_logger.h"
#include "lexical_extensions.h"

static ProducerRegistrar<MockProducer> reg1("MockProducer");
static ConsumerRegistrar<Spectrum1DEvent> reg2("1DEvent");

Setting get_profile();
Container<ConsumerMetadata> get_prototypes();

int main(int argc, char **argv)
{
  int duration = 1;
  std::string durstr;
  if (argc > 1)
    durstr = std::string (argv[1]);
  if (is_number(durstr))
    duration = std::stoi(durstr);
  if (duration < 1)
    duration = 1;

  json defs;
  defs["MockProducer"] = json();

  auto& engine = Engine::singleton();
  engine.initialize(get_profile(), defs);
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

  auto result = project->get_sink(1);
  if (result)
    DBG << "Results:\n" << result->debug("", false);

  boost::this_thread::sleep(boost::posix_time::seconds(2));

  return 0;
}

Setting get_profile()
{
  Setting default_settings({MockProducer().device_name(),
                                     SettingType::stem});
  MockProducer dummy;
  dummy.write_settings_bulk(default_settings);
  dummy.read_settings_bulk(default_settings);

  default_settings.set(Setting::integer("MockProducer/SpillInterval", 5));
  default_settings.set(Setting::integer("MockProducer/Resolution", 16));
  default_settings.set(Setting::text("MockProducer/ValName1", "energy"));
  default_settings.set(Setting::floating("MockProducer/PeakCenter", 50));
  default_settings.set(Setting::floating("MockProducer/PeakSpread", 2500));
  default_settings.set(Setting::floating("MockProducer/CountRate", 20000));
  default_settings.set(Setting::floating("MockProducer/DeadTime", 5));

  auto profile = Engine::singleton().pull_settings();
  profile.set(Setting::text("Profile description",
                            "Test profile for Mock Producer"));
  profile.branches.add(default_settings);

  return profile;
}

Container<ConsumerMetadata> get_prototypes()
{
  Container<ConsumerMetadata> prototypes;

  ConsumerMetadata ptype = ConsumerFactory::singleton().create_prototype("1DEvent");
  ptype.set_attribute(Setting::text("name", "Spectrum"));
  ptype.set_attribute(Setting::integer("resolution", 7));
  ptype.set_attribute(Setting("pattern_coinc", Pattern(1, {true})));
  ptype.set_attribute(Setting("pattern_add", Pattern(1, {true})));

  DBG << "Consumers:\n" << ptype.debug("   ", false);

  prototypes.add(ptype);
  return prototypes;
}
