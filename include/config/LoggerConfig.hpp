#pragma once
#include <string>

namespace logging {

class LoggerConfig {
public:
  static void initialize(const std::string &log_file_path);
  static void shutdown();
};

} // namespace logging
