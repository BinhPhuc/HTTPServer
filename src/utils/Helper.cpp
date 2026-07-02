#include <algorithm>
#include <chrono>
#include <format>
#include <utils/Helper.hpp>

namespace utils {
std::string lowercase(const std::string &input) {
  std::string output = input;
  std::transform(output.begin(), output.end(), output.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return output;
}

std::string rfc_date() {
  auto now = std::chrono::system_clock::now();
  std::string date = std::format("{:%a, %d %b %Y %T GMT}", now);
  return date;
}

} // namespace utils
