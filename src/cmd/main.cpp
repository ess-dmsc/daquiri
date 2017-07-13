#include "engine.h"

#include "mock_producer.h"
#include "producer_factory.h"

#include "spectrum_events_1D.h"
#include "consumer_factory.h"

#include "custom_logger.h"

static ProducerRegistrar<MockProducer> reg1("MockProducer");
static ConsumerRegistrar<Spectrum1DEvent> reg2("1DEvent");

Setting get_profile();
Container<ConsumerMetadata> get_prototypes();

int main(int argc, char **argv)
{
  json defs;
  defs["MockProducer"] = json();

  auto& engine = Engine::singleton();
  engine.initialize(get_profile(), defs);
  engine.boot();

  DBG << "\n" << engine.pull_settings().debug("   ");
  DBG << "Engine can run? " << (0 != (engine.status() & ProducerStatus::can_run));

  ProjectPtr project = ProjectPtr( new Project() );

  project->set_prototypes(get_prototypes());

  Interruptor interruptor(false);
  engine.acquire(project, interruptor, 30);
  engine.die();

  auto result = project->get_sink(1);
  if (result)
  {
    DBG << "Results:\n" << result->debug();
    auto dr = result->data_range();
    DBG << "Spectrum size = " << dr->size();

    int total{0};
    for (auto d : *dr)
      total += d.second;
    DBG << "Specturm tally = " << total;

    int nstars=100;
    for (auto d : *dr)
      if (d.second)
        DBG << d.first[0] << ": " << std::string(d.second*nstars/total,'*');
  }

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

  default_settings.set(Setting::integer("MockProducer/Resolution", 16), Match::id);
  default_settings.set(Setting::text("MockProducer/ValName1", "energy"), Match::id);
  default_settings.set(Setting::floating("MockProducer/PeakSpread", 1500), Match::id);
  default_settings.set(Setting::floating("MockProducer/CountRate", 20000), Match::id);
  default_settings.set(Setting::floating("MockProducer/DeadTime", 5), Match::id);

  Setting profile({"Profile", SettingType::stem});
  profile.branches.add(Setting::text("Profile description",
                                              "Test profile for Mock Producer"));
  profile.branches.add(default_settings);

  return profile;
}

Container<ConsumerMetadata> get_prototypes()
{
  Container<ConsumerMetadata> prototypes;

  ConsumerMetadata ptype = ConsumerFactory::singleton().create_prototype("1DEvent");
  ptype.set_attribute(Setting::text("name", "Spectrum"), false);
  ptype.set_attribute(Setting::integer("resolution", 6), false);
  ptype.set_attribute(Setting("pattern_coinc", Pattern(1, {true})), false);
  ptype.set_attribute(Setting("pattern_add", Pattern(1, {true})), false);

  DBG << "Will acquire\n" << ptype.debug("   ");

  prototypes.add(ptype);
  return prototypes;
}
