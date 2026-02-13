#include "config/LoggerConfig.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logging {

void LoggerConfig::initialize(const std::string &log_file_path) {
  try {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, true);
    
    auto logger = std::make_shared<spdlog::logger>("server", 
        spdlog::sinks_init_list{console_sink, file_sink});
    
    spdlog::set_default_logger(logger);
    
    spdlog::set_level(spdlog::level::info);
    
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] %^%l%$: %v");
    
    spdlog::info("Logger initialized successfully");
  } catch (const spdlog::spdlog_ex &ex) {
    spdlog::error("Log initialization failed: {}", ex.what());
  }
}

} // namespace logging
