#pragma once

#include <string>

class BaseReader {
protected:
  static int get_content_length(const std::string &headers);
  static bool is_multipart_request(const std::string &headers);
};
