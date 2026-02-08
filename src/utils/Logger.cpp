
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utils/Logger.hpp>

Logger &Logger::getInstance(const std::string &filename) {
  static Logger instance(filename);
  return instance;
}

Logger::Logger(const std::string &filename)
    : logFile(filename, std::ios::app), logMutex() {
  if (!logFile.is_open()) {
    std::cerr << "Error opening log file.\n";
  }
}

Logger::~Logger() { logFile.close(); }

void Logger::log(LogLevel level, const std::string &message) {
  std::lock_guard<std::mutex> lock(logMutex);
  
  time_t now = time(0);
  tm *timeinfo = localtime(&now);
  char timestamp[20];
  strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);
  std::ostringstream logEntry;
  logEntry << "[" << timestamp << "] " << levelToString(level) << ": "
           << message << "\n";

  std::cout << logEntry.str();

  if (logFile.is_open()) {
    logFile << logEntry.str();
    logFile.flush();
  }
}

std::string Logger::levelToString(LogLevel level) {
  switch (level) {
  case DEBUG:
    return "DEBUG";
  case INFO:
    return "INFO";
  case WARNING:
    return "WARNING";
  case ERROR:
    return "ERROR";
  case CRITICAL:
    return "CRITICAL";
  default:
    return "UNKNOWN";
  }
}
