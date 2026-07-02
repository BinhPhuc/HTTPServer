#pragma once

#include "handler/request/BaseReader.hpp"
#include <openssl/err.h>
#include <string>

class HttpsRequestReader : public BaseReader {
public:
  HttpsRequestReader(SSL *ssl);
  std::string read_request();

private:
  SSL *ssl;
  std::string m_buffer;
};
