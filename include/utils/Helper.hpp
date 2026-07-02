#pragma once

#include <climits>
#include <string>

namespace utils {
std::string lowercase(const std::string &input);
std::string rfc_date();
std::string generate_random_number(int min = 1, int max = INT_MAX);
} // namespace utils
