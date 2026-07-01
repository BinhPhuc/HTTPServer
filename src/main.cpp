#include "config/LoggerConfig.hpp"
#include "controller/UserController.hpp"
#include "handler/shutdown/shutdown.hpp"
#include "network/ApiRouter.hpp"
#include "network/Server.hpp"
#include "utils/Constants.hpp"
#include <csignal>
#include <spdlog/spdlog.h>

int main() {
  logging::LoggerConfig::initialize(config::SERVER_LOG_PATH);

  spdlog::info("Server is starting...");

  std::signal(SIGTERM, ShutdownHandler::signal_handler);
  std::signal(SIGINT, ShutdownHandler::signal_handler);

  {
    ApiRouter api_router;
    api_router.set_root_folder(config::SERVER_ROOT_FOLDER);

    UserController user_controller;
    user_controller.registerRoutes(api_router);

    Server server(config::PORT, api_router);
    server.start();
  }

  logging::LoggerConfig::shutdown();
  return 0;
}
