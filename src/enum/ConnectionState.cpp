#include "enum/ConnectionState.hpp"

std::string ConnectionState(ConnectionStateEnum state) {
  switch (state) {
  case ConnectionStateEnum::OK:
    return "OK";
  case ConnectionStateEnum::CLOSED:
    return "CLOSED";
  case ConnectionStateEnum::TIMEOUT:
    return "TIMEOUT";
  case ConnectionStateEnum::TOO_LARGE:
    return "TOO_LARGE";
  case ConnectionStateEnum::BAD:
    return "BAD";
  case ConnectionStateEnum::ERROR:
    return "ERROR";
  default:
    return "UNKNOWN";
  }
}
