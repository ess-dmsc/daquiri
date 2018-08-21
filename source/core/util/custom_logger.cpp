#include <core/util/custom_logger.h>

#include <fstream>

#if 0

#include <boost/core/null_deleter.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>

namespace logging = boost::log;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace date = boost::date_time;

using text_sink = sinks::asynchronous_sink<sinks::text_ostream_backend, sinks::unbounded_fifo_queue>;
using file_sink = sinks::synchronous_sink<sinks::text_file_backend>;

void CustomLogger::initLogger(std::ostream *gui_stream, std::string log_file_N)
{
  logging::add_common_attributes();

  logging::formatter format_basic =
      expr::stream << "["
                   << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S") << "] "
      //                   << severity << ": "
                   << expr::message;

  logging::formatter format_verbose =
      expr::stream << "["
                   << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y%m%d %H:%M:%S.%f")
                   << "] " << g_severity << ": "
                   << expr::message;
  
  boost::shared_ptr< logging::core > core = logging::core::get();

  // GUI
  if (gui_stream != nullptr) {
    boost::shared_ptr<sinks::text_ostream_backend> backend_gui = boost::make_shared<sinks::text_ostream_backend>();
    backend_gui->add_stream(boost::shared_ptr<std::ostream>(gui_stream, boost::null_deleter()));
    boost::shared_ptr<text_sink> sink_gui(new text_sink(backend_gui));
    sink_gui->set_formatter(format_basic);
    sink_gui->set_filter(expr::attr<SeverityLevel>("Severity") >= kInfo);
    core->add_sink(sink_gui);
  }

  // console
  boost::shared_ptr<text_sink> sink_console = boost::make_shared<text_sink>();
  boost::shared_ptr<std::ostream> console_stream(&std::cout, boost::null_deleter());
  sink_console->locked_backend()->add_stream(console_stream);
  sink_console->locked_backend()->auto_flush(true);
  sink_console->set_formatter(format_verbose);
  sink_console->set_filter(expr::attr<SeverityLevel>("Severity") >= kDebug);
  core->add_sink(sink_console);

  // file
  boost::shared_ptr<sinks::text_file_backend> backend_file = boost::make_shared<sinks::text_file_backend>(
      // file name pattern
      keywords::file_name = log_file_N,
      keywords::open_mode = (std::ios::out | std::ios::app),
      // rotate the file upon reaching 10 MiB size...
      keywords::rotation_size = 10 * 1024 * 1024
                                                                                                          );
  boost::shared_ptr<file_sink> sink_file(new file_sink(backend_file));
  sink_file->locked_backend()->auto_flush(true);
  sink_file->set_formatter(format_verbose);
  sink_file->set_filter(expr::attr<SeverityLevel >("Severity") >= kTrace);
  core->add_sink(sink_file);
}

void CustomLogger::closeLogger()
{
  DBG << "<CustomLogger> Closing logger sinks";
  logging::core::get()->remove_all_sinks();
}

#endif

#if 1

std::string ConsoleFormatter(const LogMessage &Msg) {
  static const std::vector<std::string> SevToString{"EMG", "ALR", "CRI", "ERR", "WAR", "NOTE", "INF", "DEB"};
  std::string FileName;
  std::int64_t LineNr = -1;
  for (auto &CField : Msg.additionalFields) {
    if (CField.first == "file") {
      FileName = CField.second.strVal;
    } else if (CField.first == "line") {
      LineNr = CField.second.intVal;
    }
  }
  return fmt::format("{:5}{:21}{:5} - {}", SevToString.at(int(Msg.severity)), FileName, LineNr, Msg.message);
}


void CustomLogger::initLogger(std::ostream *gui_stream, std::string log_file_N) {
  // Set-up logging before we start doing important stuff
  Log::RemoveAllHandlers();

  auto CI = new ConsoleInterface();
  CI->SetMessageStringCreatorFunction(ConsoleFormatter);
  Log::AddLogHandler(CI);

  Log::SetMinimumSeverity(Severity(5));
  if (log_file_N.size()) {
    Log::AddLogHandler(new FileInterface(log_file_N));
  }
}

void CustomLogger::closeLogger() {

}

#endif
