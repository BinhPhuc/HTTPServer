#include "enum/Phase.hpp"

std::string Phase(PhaseEnum type) {
  switch (type) {
  case PhaseEnum::HANSHAKING:
    return "HANSHAKING";
  case PhaseEnum::READING:
    return "READING";
  case PhaseEnum::WRITING:
    return "WRITING";
  case PhaseEnum::CLOSING:
    return "CLOSING";
  default:
    return "UNKNOWN";
  }
}
