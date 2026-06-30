#pragma once
#include <string>

namespace config {
constexpr int PORT = 8080;
const std::string SERVER_LOG_PATH =
    std::string(PROJECT_ROOT_DIR) + "/logs/server.log";
const std::string SERVER_ROOT_FOLDER = "public";
constexpr int MAX_CONNECTIONS = 10;
const std::string CERT_FILE = std::string(PROJECT_ROOT_DIR) + "/certs/cert.pem";
const std::string KEY_FILE = std::string(PROJECT_ROOT_DIR) + "/certs/key.pem";
const int MAX_BODY_SIZE = 1 * 1024 * 1024;    // MAX_BODY_SIZE = 1MB
const int MAX_UPLOAD_SIZE = 10 * 1024 * 1024; // MAX_UPLOAD_SIZE = 10MB
}; // namespace config
