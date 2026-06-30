#include "handler/request/BaseReader.hpp"
#include <charconv>
#include <system_error>

int BaseReader::get_content_length(const std::string &headers) {
  std::string key = "Content-Length:";
  size_t pos = headers.find(key);
  if (pos == std::string::npos) {
    return 0;
  }
  pos += key.length();
  while (pos < headers.length() &&
         (headers[pos] == ' ' || headers[pos] == '\t')) {
    pos++;
  }
  size_t end_pos = headers.find("\r\n", pos);
  std::string value = headers.substr(pos, end_pos - pos);
  int content_length = 0;
  auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(),
                                   content_length);
  if (ec == std::errc{}) {
    return content_length;
  } else if (ec == std::errc::invalid_argument) {
    return 0;
  } else if (ec == std::errc::result_out_of_range) {
    return 0;
  }
  return content_length;
}

bool BaseReader::is_multipart_request(const std::string &headers) {
  std::string key = "Content-Type:";
  size_t pos = headers.find(key);
  if (pos == std::string::npos) {
    return false;
  }
  pos += key.length();
  while (pos < headers.length() &&
         (headers[pos] == ' ' || headers[pos] == '\t')) {
    pos++;
  }
  size_t end_pos = headers.find("\r\n", pos);
  std::string value = headers.substr(pos, end_pos - pos);
  return value.find("multipart/form-data") != std::string::npos;
}
