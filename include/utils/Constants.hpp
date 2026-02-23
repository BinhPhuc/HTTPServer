#pragma once
#include <string>

namespace config {
constexpr int PORT = 8080;
const std::string SERVER_LOG_PATH = std::string(PROJECT_ROOT_DIR) + "/logs/server.log";
const std::string SERVER_ROOT_FOLDER = "public";
constexpr int MAX_CONNECTIONS = 10;
}; // namespace config
