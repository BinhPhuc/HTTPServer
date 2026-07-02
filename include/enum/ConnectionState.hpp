#pragma once

#include <string>

enum class ConnectionStateEnum { OK, CLOSED, TIMEOUT, TOO_LARGE, BAD, ERROR };

std::string ConnectionState(ConnectionStateEnum state);
