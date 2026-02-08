#include "network/Server.hpp"
#include "utils/Constants.hpp"
#include "utils/Logger.hpp"

int main() {
  Logger::getInstance(config::SERVER_LOG_PATH)
      .log(INFO, "Server is starting...");
  Server server(8080);
  server.start();
  return 0;
}
