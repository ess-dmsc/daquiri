#include <core/util/custom_logger.h>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <fstream>
#include <date/date.h>

// \todo use fractional seconds in file

//class OstreamInterface : public Log::BaseLogHandler
//{
// public:
//  OstreamInterface(std::ostream* gui_stream, const size_t maxQueueLength = 100)
//      : BaseLogHandler(maxQueueLength), outStream(gui_stream)
//        , ostreamThread(&OstreamInterface::ThreadFunction, this)
//  {}
//
//  ~OstreamInterface()
//  {
//    ExitThread();
//  }
//
// protected:
//  void ExitThread()
//  {
//    Log::LogMessage exitMsg;
//    exitMsg.MessageString = "exit";
//    exitMsg.ProcessId = -1;
//    addMessage(exitMsg);
//    if (ostreamThread.joinable())
//    {
//      ostreamThread.join();
//    }
//  }
//
//  void ThreadFunction()
//  {
//    Log::LogMessage tmpMsg;
//    while (true)
//    {
//      MessageQueue.wait_and_pop(tmpMsg);
//      if (std::string("exit") == tmpMsg.MessageString and -1 == tmpMsg.ProcessId)
//      {
//        break;
//      }
//      if (outStream)
//      {
//        *outStream << messageToString(tmpMsg) << std::endl;
//      }
//    }
//  }
//
//  std::ostream* outStream{nullptr};
//  std::thread ostreamThread;
//};

namespace CustomLogger
{

void initLogger(const spdlog::level::level_enum& LoggingLevel,
                const std::string& log_file_name,
                std::ostream* gui_stream)
{
  spdlog::shutdown();

  std::vector<spdlog::sink_ptr> sinks;

  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] [processID: %P]: %v");
  sinks.push_back(console_sink);

  if (!log_file_name.empty())
  {
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_name);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%l] [processID: %P]: %v");
    sinks.push_back(file_sink);
  }

//  if (gui_stream)
//  {
//    auto GI = new OstreamInterface(gui_stream);
//    GI->setMessageStringCreatorFunction(GuiFormatter);
//    Log::AddLogHandler(GI);
//  }

  auto combined_logger = std::make_shared<spdlog::logger>(
      "daquiri_logger", begin(sinks), end(sinks));
  combined_logger->set_level(LoggingLevel);
  combined_logger->flush_on(spdlog::level::err);
  spdlog::set_default_logger(combined_logger);
}

SharedLogger getLogger()
{
  return spdlog::get("daquiri_logger");
}

void closeLogger()
{
  // Flush all *registered* loggers using a worker thread every 3 seconds.
  // note: registered loggers *must* be thread safe for this to work correctly!
//  spdlog::flush_every(std::chrono::seconds(3));

  // Release all spdlog resources, and drop all loggers in the registry.
  // This is optional (only mandatory if using windows + async log).
  spdlog::shutdown();
}

}