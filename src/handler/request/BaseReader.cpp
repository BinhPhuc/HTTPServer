#include "handler/request/BaseReader.hpp"

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
  return std::stoi(value);
}
