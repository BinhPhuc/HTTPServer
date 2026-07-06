#include "config/LoggerConfig.hpp"
#include "controller/UserController.hpp"
#include "handler/shutdown/shutdown.hpp"
#include "network/ApiRouter.hpp"
#include "network/Server.hpp"
#include "utils/Constants.hpp"
#include <csignal>
#include <cstddef>
#include <spdlog/spdlog.h>
#include <string>
#include <string_view>

namespace {
size_t parse_worker_count(int argc, char **argv) {
  constexpr std::string_view prefix = "--worker=";
  for (int i = 1; i < argc; ++i) {
    std::string_view arg(argv[i]);
    if (arg.substr(0, prefix.size()) != prefix) {
      continue;
    }
    std::string value(arg.substr(prefix.size()));
    try {
      long count = std::stol(value);
      if (count <= 0) {
        spdlog::warn("Ignoring non-positive --worker value '{}'.", value);
        return 0;
      }
      return static_cast<size_t>(count);
    } catch (const std::exception &) {
      spdlog::warn("Ignoring invalid --worker value '{}'.", value);
      return 0;
    }
  }
  return 0;
}

bool parse_keep_alive(int argc, char **argv) {
  constexpr std::string_view prefix = "--keep-alive=";
  for (int i = 1; i < argc; ++i) {
    std::string_view arg(argv[i]);
    if (arg.substr(0, prefix.size()) != prefix) {
      continue;
    }
    std::string_view value = arg.substr(prefix.size());
    if (value == "false" || value == "0") {
      return false;
    }
    if (value == "true" || value == "1") {
      return true;
    }
    spdlog::warn("Ignoring invalid --keep-alive value '{}'.",
                 std::string(value));
    return true;
  }
  return true;
}
} // namespace

int main(int argc, char **argv) {
  logging::LoggerConfig::initialize(config::SERVER_LOG_PATH);

  spdlog::info("Server is starting...");

  std::signal(SIGTERM, ShutdownHandler::signal_handler);
  std::signal(SIGINT, ShutdownHandler::signal_handler);

  size_t num_workers = parse_worker_count(argc, argv);
  bool keep_alive = parse_keep_alive(argc, argv);

  {
    ApiRouter api_router;
    api_router.set_root_folder(config::SERVER_ROOT_FOLDER);

    UserController user_controller;
    user_controller.registerRoutes(api_router);

    Server server(config::PORT, api_router, num_workers, keep_alive);
    server.start();
  }

  logging::LoggerConfig::shutdown();
  return 0;
}
