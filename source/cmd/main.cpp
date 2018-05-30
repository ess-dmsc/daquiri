#include <CLI11.hpp>

#include "engine.h"

#include "custom_logger.h"
#include "lexical_extensions.h"

#include "consumers_autoreg.h"
#include "producers_autoreg.h"

#include "producer_factory.h"
#include "consumer_factory.h"

#include "custom_timer.h"

#include "h5json.h"

#include "json_file.h"

#include <signal.h>

using namespace DAQuiri;

std::atomic<bool> interruptor(false);
void term_key(int /*sig*/)
{
  interruptor.store(true);
}

int main(int argc, char** argv)
{
  signal(SIGINT, term_key);

  CustomLogger::initLogger(nullptr, "acquire");
  hdf5::error::Singleton::instance().auto_print(false);
  producers_autoreg();
  consumers_autoreg();

  CLI::App app {"acquire -- when having a GUI is just too much"};

  uint64_t duration {1};
  app.add_option("-t,--time", duration, "How long shall we run?", true)
      ->check(CLI::Range(uint64_t(1), std::numeric_limits<uint64_t>::max()));

  std::string profile_file;
  app.add_option("-i,--input", profile_file, "DAQ producer config profile")
      ->check(CLI::ExistingFile);

  std::string consumers;
  app.add_option("-o,--output", consumers, "DAQ consumer prototype project")
      ->check(CLI::ExistingFile);

  bool verbose {false};
  app.add_flag("-v,--verbose", verbose, "Print results");

  std::string save_h5;
  app.add_option("-s,--save", save_h5, "Save to h5 file");

  std::string save_csv;
  app.add_option("-c,--save_csv", save_csv, "Save to multiple csv files");

  CLI11_PARSE(app, argc, argv);

  auto& engine = Engine::singleton();

  nlohmann::json profile;
  try
  {
    profile = from_json_file(profile_file);
    engine.initialize(profile);
  }
  catch (std::exception& e)
  {
    ERR << "Failed to read engine config file '" << profile_file << "'. "
        << "Json parsing error:\n"
        << hdf5::error::print_nested(e, 1);
    return EXIT_FAILURE;
  }

  if (verbose)
    INFO << "\n" << engine.settings().debug("   ", false);

  if (0 == (engine.status() & ProducerStatus::can_boot))
  {
    ERR << "Bad DAQ config. Engine cannot boot.";
    return EXIT_FAILURE;
  }

  engine.boot();

  if (0 == (engine.status() & ProducerStatus::can_run))
  {
    ERR << "Bad DAQ config. Engine cannot run.";
    return EXIT_FAILURE;
  }

  if (!hdf5::file::is_hdf5_file(consumers))
  {
    ERR << "Supplied project prototype '" << consumers << "' is not an hdf5 file. "
        << "No consumer prototypes specified. Cannot acquire data.";
    return EXIT_FAILURE;
  }

  ProjectPtr project = ProjectPtr(new Project());
  project->open(consumers);

  if (verbose)
    INFO << "Project before DAQ run:\n" << *project;

  engine.acquire(project, interruptor, duration);
  engine.die();

  if (verbose)
    INFO << "Project after DAQ run:\n" << *project;

  if (!save_h5.empty())
  {
    INFO << "Saving h5 to " << save_h5;
    project->save(save_h5);
  }

  if (!save_csv.empty())
  {
    INFO << "Saving csv to " << save_csv;
    project->save_split(save_csv);
  }

  return EXIT_SUCCESS;
}
