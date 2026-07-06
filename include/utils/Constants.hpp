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
constexpr int MAX_BODY_SIZE = 1 * 1024 * 1024;
constexpr int MAX_UPLOAD_SIZE = 10 * 1024 * 1024;
constexpr int MAX_HEADER_SIZE = 8192;
constexpr int MAX_REQUEST_SIZE = MAX_HEADER_SIZE + MAX_BODY_SIZE;
constexpr int SOCKET_TIMEOUT = 5;
};
