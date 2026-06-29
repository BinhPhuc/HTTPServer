#pragma once

#include "handler/request/BaseReader.hpp"
#include <openssl/err.h>
#include <string>

class HttpsRequestReader : public BaseReader {
public:
  static std::string read_request(SSL *ssl);
};
