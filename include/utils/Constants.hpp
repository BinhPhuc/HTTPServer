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
constexpr int MAX_BODY_SIZE = 1 * 1024 * 1024;    // MAX_BODY_SIZE = 1MB
constexpr int MAX_UPLOAD_SIZE = 10 * 1024 * 1024; // MAX_UPLOAD_SIZE = 10MB
constexpr int MAX_HEADER_SIZE = 8192; // MAX_HEADER_SIZE = 8KB -> unused
constexpr int MAX_REQUEST_SIZE =
    MAX_HEADER_SIZE + MAX_BODY_SIZE; // MAX_REQUEST_SIZE = 8KB + 1MB -> unused
constexpr int SOCKET_TIMEOUT = 5;    // SOCKET_TIMEOUT = 5 seconds
}; // namespace config
