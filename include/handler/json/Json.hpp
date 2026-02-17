#pragma once

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class Json {
public:
  template <typename T>
  static std::string stringify(const T &data) { // Object to JSON string
    if constexpr (std::is_same_v<T, std::string>) {
      return data;
    } else if constexpr (std::is_same_v<T, const char *>) {
      return std::string(data);
    } else {
      json j = data;
      return j.dump();
    }
  }
};
