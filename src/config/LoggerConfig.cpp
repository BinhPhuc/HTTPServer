#include "config/LoggerConfig.hpp"
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logging {

void LoggerConfig::initialize(const std::string &log_file_path) {
  try {
    // Initialize async thread pool: queue size 8192, 1 background thread
    spdlog::init_thread_pool(8192, 1);
    
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_path, true);
    
    auto logger = std::make_shared<spdlog::async_logger>(
        "server",
        spdlog::sinks_init_list{console_sink, file_sink},
        spdlog::thread_pool(),
        spdlog::async_overflow_policy::block);
    
    spdlog::set_default_logger(logger);
    
    spdlog::set_level(spdlog::level::info);
    
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S] %^%l%$: %v");
    
    spdlog::info("Async logger initialized successfully");
  } catch (const spdlog::spdlog_ex &ex) {
    spdlog::error("Log initialization failed: {}", ex.what());
  }
}

void LoggerConfig::shutdown() {
  spdlog::info("Shutting down logger...");
  spdlog::shutdown();
}

} // namespace logging
