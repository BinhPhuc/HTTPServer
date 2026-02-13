#include "config/LoggerConfig.hpp"
#include "network/Server.hpp"
#include "utils/Constants.hpp"
#include <spdlog/spdlog.h>

int main() {
  logging::LoggerConfig::initialize(config::SERVER_LOG_PATH);

  spdlog::info("Server is starting...");

  Server server(8080);

  server.start();
  return 0;
}
