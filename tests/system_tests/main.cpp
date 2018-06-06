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

void add_histogram1d(ProjectPtr);
void add_histogram2d(ProjectPtr);
void add_histogram3d(ProjectPtr);
void add_image2d(ProjectPtr);
void add_tof2d(ProjectPtr);
void add_time2d(ProjectPtr);

void add_prebinned1d(ProjectPtr);
void add_tof1d(ProjectPtr);
void add_time_domain(ProjectPtr);
void add_stats_scalar(ProjectPtr);

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
                  std::string name, double center, double spread);

ProjectPtr get_project()
{
  ProjectPtr project = ProjectPtr(new Project());
  add_histogram1d(project);
  add_histogram2d(project);
  add_image2d(project);
  add_tof2d(project);
  add_time2d(project);
  add_histogram3d(project);
  add_prebinned1d(project);
  add_tof1d(project);
  add_time_domain(project);
  add_stats_scalar(project);
  return project;
}

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

  auto project = get_project();

  Interruptor interruptor(false);
  engine.acquire(project, interruptor, duration);
  engine.die();

  project->save_split("./results_split");
  project->save("./results.h5");

  ProjectPtr project2 = ProjectPtr(new Project());
  project2->open("./results.h5");

  std::stringstream ss1, ss2;
  ss1 << *project;
  ss2 << *project2;

  DBG << "Acquired:\n" << ss1.str();

//  DBG << "Loaded:\n" << ss2.str();

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

void add_histogram1d(ProjectPtr project)
{
  auto h1d = ConsumerFactory::singleton().create_type("Histogram 1D");
  h1d->set_attribute(Setting::text("stream_id", "exy"));
  h1d->set_attribute(Setting::text("value_id", "x"));
  h1d->set_attribute(Setting::integer("downsample", 10));
  project->add_consumer(h1d);
}

void add_histogram2d(ProjectPtr project)
{
  auto h2d = ConsumerFactory::singleton().create_type("Histogram 2D");
  h2d->set_attribute(Setting::text("stream_id", "exy"));

  auto x_name = Setting::text("value_id", "x");
  x_name.set_indices({0});
  h2d->set_attribute(x_name);

  auto y_name = Setting::text("value_id", "y");
  y_name.set_indices({1});
  h2d->set_attribute(y_name);

  auto ds = Setting::integer("downsample", 11);
  ds.set_indices({0});
  h2d->set_attribute(ds);
  ds.set_indices({1});
  h2d->set_attribute(ds);

  project->add_consumer(h2d);
}

void add_image2d(ProjectPtr project)
{
  auto im2d = ConsumerFactory::singleton().create_type("Image 2D");
  im2d->set_attribute(Setting::text("stream_id", "exy"));

  auto x_name = Setting::text("value_id", "x");
  x_name.set_indices({0});
  im2d->set_attribute(x_name);

  auto y_name = Setting::text("value_id", "y");
  y_name.set_indices({1});
  im2d->set_attribute(y_name);

  auto intensity_name = Setting::text("value_id", "energy");
  intensity_name.set_indices({2});
  im2d->set_attribute(intensity_name);

  auto ds = Setting::integer("downsample", 10);
  ds.set_indices({0});
  im2d->set_attribute(ds);
  ds.set_indices({1});
  im2d->set_attribute(ds);

  project->add_consumer(im2d);
}

void add_tof2d(ProjectPtr project)
{
  auto tof2d = ConsumerFactory::singleton().create_type("Time of Flight 2D");
  tof2d->set_attribute(Setting::text("stream_id", "exy"));
  tof2d->set_attribute(Setting::floating("time_resolution", 5));
  tof2d->set_attribute(Setting::integer("time_units", 6));

  tof2d->set_attribute(Setting::text("value_id", "x"));
  tof2d->set_attribute(Setting::integer("downsample", 10));

  project->add_consumer(tof2d);
}

void add_time2d(ProjectPtr project)
{
  auto time2d = ConsumerFactory::singleton().create_type("TimeSpectrum 2D");
  time2d->set_attribute(Setting::text("stream_id", "exy"));
  time2d->set_attribute(Setting::floating("time_resolution", 20));
  time2d->set_attribute(Setting::integer("time_units", 6));

  time2d->set_attribute(Setting::text("value_id", "x"));
  time2d->set_attribute(Setting::integer("downsample", 10));

  project->add_consumer(time2d);
}

void add_histogram3d(ProjectPtr project)
{
  auto h3d = ConsumerFactory::singleton().create_type("Histogram 3D");
  h3d->set_attribute(Setting::text("stream_id", "exy"));

  auto x_name = Setting::text("value_id", "x");
  x_name.set_indices({0});
  h3d->set_attribute(x_name);

  auto y_name = Setting::text("value_id", "y");
  y_name.set_indices({1});
  h3d->set_attribute(y_name);

  auto z_name = Setting::text("value_id", "z");
  z_name.set_indices({2});
  h3d->set_attribute(z_name);

  auto ds = Setting::integer("downsample", 12);
  ds.set_indices({0});
  h3d->set_attribute(ds);
  ds.set_indices({1});
  h3d->set_attribute(ds);
  ds.set_indices({2});
  h3d->set_attribute(ds);

  project->add_consumer(h3d);
}

void add_prebinned1d(ProjectPtr project)
{

  auto h1d = ConsumerFactory::singleton().create_type("Prebinned 1D");
  h1d->set_attribute(Setting::text("stream_id", "exy"));
  h1d->set_attribute(Setting::text("trace_id", "x"));
  project->add_consumer(h1d);
}

void add_tof1d(ProjectPtr project)
{
  auto tof1d = ConsumerFactory::singleton().create_type("Time of Flight 1D");
  tof1d->set_attribute(Setting::text("stream_id", "exy"));
  tof1d->set_attribute(Setting::floating("time_resolution", 5));
  tof1d->set_attribute(Setting::integer("time_units", 6));
  project->add_consumer(tof1d);
}

void add_time_domain(ProjectPtr project)
{
  auto act = ConsumerFactory::singleton().create_type("Time-Activity 1D");
  act->set_attribute(Setting::text("stream_id", "exy"));
  act->set_attribute(Setting::floating("time_resolution", 25));
  act->set_attribute(Setting::integer("time_units", 6));
  project->add_consumer(act);
}

void add_stats_scalar(ProjectPtr project)
{
  auto scal = ConsumerFactory::singleton().create_type("Stats Scalar");
  scal->set_attribute(Setting::text("stream_id", "exy"));
  scal->set_attribute(Setting::text("what_stats", "native_time"));
  project->add_consumer(scal);
}
