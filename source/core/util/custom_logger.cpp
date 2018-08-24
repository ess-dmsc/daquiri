#include <core/util/custom_logger.h>

#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <graylog_logger/FileInterface.hpp>
#include <graylog_logger/ConsoleInterface.hpp>
#pragma GCC diagnostic pop

#include <ciso646>
#include <ctime>

// \todo use fractional seconds in console and file

class OstreamInterface : public BaseLogHandler {
public:
  OstreamInterface(std::ostream *gui_stream, const size_t maxQueueLength = 100)
  : BaseLogHandler(maxQueueLength), outStream(gui_stream)
  , ostreamThread(&OstreamInterface::ThreadFunction, this) {}
  ~OstreamInterface() {
    ExitThread();
  }

protected:
  void ExitThread() {
    LogMessage exitMsg;
    exitMsg.message = "exit";
    exitMsg.processId = -1;
    AddMessage(exitMsg);
    if (ostreamThread.joinable()) {
      ostreamThread.join();
    }
  }
  void ThreadFunction() {
    LogMessage tmpMsg;
    while (true) {
      logMessages.wait_and_pop(tmpMsg);
      if (std::string("exit") == tmpMsg.message and -1 == tmpMsg.processId) {
        break;
      }
      if (outStream) {
        *outStream << MsgStringCreator(tmpMsg) << std::endl;
      }
    }
  }
  std::thread ostreamThread;
  std::ostream *outStream {nullptr};
};

std::string ConsoleFormatter(const LogMessage &Msg) {
  static const std::vector<std::string> SevToString{"EMG", "ALR", "CRI", "ERR", "WAR", "NOTE", "INF", "DBG"};

  std::time_t cTime = std::chrono::system_clock::to_time_t(Msg.timestamp);
  char timeBuffer[50];
  size_t bytes = std::strftime(timeBuffer, 50, "%F %T", std::localtime(&cTime));

  std::string extras;
  for (auto &CField : Msg.additionalFields) {
    if (CField.first == "file") {
      extras += fmt::format(" {:21}", CField.second.strVal);
    } else if (CField.first == "line") {
      extras += fmt::format(":{}", CField.second.intVal);
    }
  }
  return fmt::format("{} {}{} {}",
      std::string(timeBuffer, bytes),
      SevToString.at(static_cast<uint32_t>(Msg.severity)),
      extras, Msg.message);
}


void CustomLogger::initLogger(Severity severity, std::ostream *gui_stream, std::string log_file_N) {
  // Set-up logging before we start doing important stuff
  Log::RemoveAllHandlers();

  Log::SetMinimumSeverity(severity);

  auto CI = new ConsoleInterface();
  CI->SetMessageStringCreatorFunction(ConsoleFormatter);
  Log::AddLogHandler(CI);

  if (log_file_N.size()) {
    Log::AddLogHandler(new FileInterface(log_file_N));
  }

  if (gui_stream) {
    Log::AddLogHandler(new OstreamInterface(gui_stream));
  }
}

void CustomLogger::closeLogger() {

}
