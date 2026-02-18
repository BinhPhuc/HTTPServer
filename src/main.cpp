#include "config/LoggerConfig.hpp"
#include "controller/UserController.hpp"
#include "network/ApiRouter.hpp"
#include "network/Server.hpp"
#include "utils/Constants.hpp"
#include <spdlog/spdlog.h>

int main() {
  logging::LoggerConfig::initialize(config::SERVER_LOG_PATH);
  spdlog::info("Server is starting...");

  ApiRouter api_router;
  api_router.set_root_folder(config::SERVER_ROOT_FOLDER);

  UserController user_controller;
  user_controller.registerRoutes(api_router);

  Server server(8080, api_router);
  server.start();
  return 0;
}
