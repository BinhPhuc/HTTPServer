#pragma once
#include <fstream>
#include <mutex>
#include "Constants.hpp"

enum LogLevel { DEBUG, INFO, WARNING, ERROR, CRITICAL };

class Logger {
public:
  static Logger &getInstance(const std::string &filename);
  void log(LogLevel level, const std::string &message);

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

private:
  Logger(const std::string &filename);
  ~Logger();
  std::ofstream logFile;
  std::mutex logMutex;
  std::string levelToString(LogLevel level);
};
