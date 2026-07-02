#pragma once

#include "handler/request/BaseReader.hpp"
#include <openssl/err.h>
#include <string>

class HttpsRequestReader : public BaseReader {
public:
  HttpsRequestReader(SSL *ssl);
  HttpsRequestReader(const HttpsRequestReader &) = delete;
  HttpsRequestReader &operator=(const HttpsRequestReader &) = delete;
  std::string read_request();

private:
  SSL *ssl;
  std::string m_buffer;
};
