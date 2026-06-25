#pragma once

#include <openssl/err.h>
#include <string>

class HttpsRequestReader {
public:
  static int get_content_length(const std::string &headers);
  static std::string read_request(SSL *ssl);
};
