#pragma once

#include "../Types.h"
#include <cstdio>
#include <ctime>
#include <raylib.h>
#include <string>
#include <vector>

namespace rnf {

struct LogEntry {
  LogLevel level;
  std::string message;
  std::string timestamp;
};

class Logger {
public:
  // NOTE: Logs a trace message.
  static void trace(const std::string &msg) { log(LogLevel::Trace, msg); }

  // NOTE: Logs a debug message.
  static void debug(const std::string &msg) { log(LogLevel::Debug, msg); }

  // NOTE: Logs an info message.
  static void info(const std::string &msg) { log(LogLevel::Info, msg); }

  // NOTE: Logs a warning message.
  static void warn(const std::string &msg) { log(LogLevel::Warning, msg); }

  // NOTE: Logs an error message.
  static void error(const std::string &msg) { log(LogLevel::Error, msg); }

  // NOTE: Logs a critical message.
  static void critical(const std::string &msg) { log(LogLevel::Critical, msg); }

  // NOTE: Sets the minimum log level to display.
  static void setLevel(LogLevel level) { s_level = level; }

  // NOTE: Enables or disables console output.
  static void setConsoleOutput(bool enabled) { s_console = enabled; }

  // NOTE: Returns the list of stored log entries.
  static const std::vector<LogEntry> &entries() { return s_entries; }

  // NOTE: Clears all stored logs.
  static void clear() { s_entries.clear(); }

private:
  static void log(LogLevel level, const std::string &msg) {
    if (level < s_level)
      return;

    LogEntry entry;
    entry.level = level;
    entry.message = msg;
    entry.timestamp = currentTime();

    if (s_entries.size() >= 200)
      s_entries.erase(s_entries.begin());

    s_entries.push_back(entry);

    if (s_console) {
      const char *prefix = levelPrefix(level);
      std::printf("[%s] %s %s\n", entry.timestamp.c_str(), prefix, msg.c_str());
    }
  }

  static const char *levelPrefix(LogLevel level) {
    switch (level) {
    case LogLevel::Trace:
      return "[TRACE]";
    case LogLevel::Debug:
      return "[DEBUG]";
    case LogLevel::Info:
      return "[INFO] ";
    case LogLevel::Warning:
      return "[WARN] ";
    case LogLevel::Error:
      return "[ERROR]";
    case LogLevel::Critical:
      return "[CRIT] ";
    default:
      return "[?]   ";
    }
  }

  static std::string currentTime() {
    std::time_t t = std::time(nullptr);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));
    return buf;
  }

  inline static LogLevel s_level = LogLevel::Info;
  inline static bool s_console = true;
  inline static std::vector<LogEntry> s_entries;
};

} // namespace rnf
