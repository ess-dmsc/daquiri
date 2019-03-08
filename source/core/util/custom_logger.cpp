#include <core/util/custom_logger.h>

#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <graylog_logger/FileInterface.hpp>
#include <graylog_logger/ConsoleInterface.hpp>
#pragma GCC diagnostic pop

#include <date/date.h>

// \todo use fractional seconds in file

class OstreamInterface : public Log::BaseLogHandler {
public:
  OstreamInterface(std::ostream *gui_stream, const size_t maxQueueLength = 100)
      : BaseLogHandler(maxQueueLength), outStream(gui_stream)
      , ostreamThread(&OstreamInterface::ThreadFunction, this) {}
  ~OstreamInterface() {
    ExitThread();
  }

protected:
  void ExitThread() {
    Log::LogMessage exitMsg;
    exitMsg.MessageString = "exit";
    exitMsg.ProcessId = -1;
    addMessage(exitMsg);
    if (ostreamThread.joinable()) {
      ostreamThread.join();
    }
  }
  void ThreadFunction() {
    Log::LogMessage tmpMsg;
    while (true) {
      MessageQueue.wait_and_pop(tmpMsg);
      if (std::string("exit") == tmpMsg.MessageString and -1 == tmpMsg.ProcessId) {
        break;
      }
      if (outStream) {
        *outStream << messageToString(tmpMsg) << std::endl;
      }
    }
  }
  std::ostream *outStream{nullptr};
  std::thread ostreamThread;
};

std::string ConsoleFormatter(const Log::LogMessage &Msg) {
  static const std::vector<std::string> SevToString{"EMG", "ALR", "CRI", "ERR", "WAR", "NOTE", "INF", "DBG"};

  std::string extras;
  for (auto &CField : Msg.AdditionalFields) {
    if (CField.first == "file") {
      extras += fmt::format(" {:21}", CField.second.strVal);
    } else if (CField.first == "line") {
      extras += fmt::format(":{}", CField.second.intVal);
    }
  }
  return fmt::format("{} {}{} {}",
                     date::format("%F %T", date::floor<std::chrono::microseconds>(Msg.Timestamp)),
                     SevToString.at(static_cast<uint32_t>(Msg.SeverityLevel)),
                     extras, Msg.MessageString);
}

std::string GuiFormatter(const Log::LogMessage &Msg) {
  static const std::vector<std::string> SevToString{"EMG", "ALR", "CRI", "ERR", "WAR", "NOTE", "INF", "DBG"};

  return fmt::format("{} {}",
                     date::format("%F %T", date::floor<std::chrono::microseconds>(Msg.Timestamp)),
                     Msg.MessageString);
}

void CustomLogger::initLogger(Log::Severity severity, std::ostream *gui_stream, std::string log_file_N) {
  // Set-up logging before we start doing important stuff
  Log::RemoveAllHandlers();

  Log::SetMinimumSeverity(severity);

  auto CI = new Log::ConsoleInterface();
  CI->setMessageStringCreatorFunction(ConsoleFormatter);
  Log::AddLogHandler(CI);

  if (log_file_N.size()) {
    Log::AddLogHandler(new Log::FileInterface(log_file_N));
  }

  if (gui_stream) {
    auto GI = new OstreamInterface(gui_stream);
    GI->setMessageStringCreatorFunction(GuiFormatter);
    Log::AddLogHandler(GI);
  }
}

void CustomLogger::closeLogger() {

}
