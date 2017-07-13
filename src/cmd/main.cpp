#include "engine.h"

#include "mock_producer.h"
#include "producer_factory.h"

#include "spectrum_events_1D.h"
#include "consumer_factory.h"

#include "custom_logger.h"

static ProducerRegistrar<MockProducer> reg1("MockProducer");
static ConsumerRegistrar<Spectrum1DEvent> reg2("1DEvent");

int main(int argc, char **argv)
{
  Setting default_settings({MockProducer().device_name(),
                                     SettingType::stem});
  MockProducer dummy;
  dummy.write_settings_bulk(default_settings);
  dummy.read_settings_bulk(default_settings);

  Setting profile({"Profile", SettingType::stem});
  profile.branches.add(Setting::text("Profile description",
                                              "Test profile for Mock Producer"));
  profile.branches.add(default_settings);

  json defs;
  defs["MockProducer"] = json();

  auto& engine = Engine::singleton();
  engine.initialize(profile, defs);
  engine.boot();

  DBG << "\n" << engine.pull_settings().debug();
  DBG << "Engine can run? " << (0 != (engine.status() & ProducerStatus::can_run));

  auto project = ProjectPtr( new Project() );
  Container<ConsumerMetadata> prototypes;
  prototypes.add_a(ConsumerFactory::singleton().create_prototype("1DEvent"));
  project->set_prototypes(prototypes);

  Interruptor interruptor(false);
  engine.acquire(project, interruptor, 30);

  engine.die();

  return 0;
}
