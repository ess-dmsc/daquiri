#include <CLI/CLI.hpp>

#include <core/engine.h>

#include <core/util/custom_logger.h>

#include <consumers/consumers_autoreg.h>
#include <producers/producers_autoreg.h>

#include <core/util/json_file.h>

#include <signal.h>
#include <build_time.h>

using namespace DAQuiri;

std::atomic<bool> interruptor(false);
void term_key(int /*sig*/) {
  interruptor.store(true);
}

struct AcquireOptions {
  CLI::App app{"acquire: when you can't afford a daquiri"};

  uint64_t duration{0};
  std::string profile_file;
  std::string consumers;
  bool verbose{false};
  std::string save_h5;
  std::string save_csv;

  AcquireOptions() {
    app.add_option("-t,--time", duration, "How long shall we run?", true)
        ->check(CLI::Range(uint64_t(1), std::numeric_limits<uint64_t>::max()));
    app.add_option("-i,--input", profile_file, "DAQ producer config profile")
        ->check(CLI::ExistingFile);
    app.add_option("-o,--output", consumers, "DAQ consumer prototype project")
        ->check(CLI::ExistingFile);
    app.add_flag("-v,--verbose", verbose, "Print results");
    app.add_option("-s,--save", save_h5, "Save to h5 file");
    app.add_option("-c,--save_csv", save_csv, "Save to multiple csv files");
  }
};

int main(int argc, char **argv) {
  AcquireOptions opts;
  CLI11_PARSE(opts.app, argc, argv);

  CustomLogger::initLogger(Log::Severity::Info, nullptr, "acquire.log");
  hdf5::error::Singleton::instance().auto_print(false);
  producers_autoreg();
  consumers_autoreg();

  signal(SIGINT, term_key);

  INFO("BuildInfo.branch: {}", BI_GIT_BRANCH);
  INFO("BuildInfo.git_hash: {}", BI_GIT_HASH);
  INFO("BuildInfo.user@host: {}@{}", BI_USERNAME, BI_HOSTNAME);
  INFO("BuildInfo.system: {} {}", BI_SYSTEM, BI_PROCESSOR);
  INFO("BuildInfo.build_time: {}", BUILD_TIME);

  auto &engine = Engine::singleton();

  if (!opts.profile_file.empty()) {
    nlohmann::json profile;
    try {
      profile = from_json_file(opts.profile_file);
      engine.initialize(profile);
    }
    catch (std::exception &e) {
      ERR("Failed to read engine config file '{}'. Json parsing error:\n{}",
          opts.profile_file, hdf5::error::print_nested(e, 1));
      return EXIT_FAILURE;
    }
  }

  ProjectPtr project = ProjectPtr(new Project());

  if (!opts.consumers.empty()) {
    if (!hdf5::file::is_hdf5_file(opts.consumers)) {
      ERR("Supplied project prototype '{}' is not an hdf5 file. "
          "No consumer prototypes specified. Cannot acquire data.", opts.consumers);
      return EXIT_FAILURE;
    }
    project->open(opts.consumers);
  }

  engine.boot();

  if (opts.verbose) {
    INFO("Engine status:\n{}", engine.settings().debug("   ", false));
    std::stringstream ss;
    ss << *project;
    INFO("Project before DAQ run:\n{}", ss.str());
  }

  engine.acquire(project, interruptor, opts.duration);

  if (opts.verbose)
  {
    std::stringstream ss;
    ss << *project;
    INFO("Project after DAQ run:\n{}", ss.str());
  }

  if (!opts.save_h5.empty()) {
    INFO("Saving h5 to {}", opts.save_h5);
    project->save(opts.save_h5);
  }

  if (!opts.save_csv.empty()) {
    INFO("Saving csv to {}", opts.save_csv);
    project->save_split(opts.save_csv);
  }

  engine.die();

  return EXIT_SUCCESS;
}
