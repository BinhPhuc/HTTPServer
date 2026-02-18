#include "enum/FileEnum.hpp"

std::string FileStatusMessage(FileStatusEnum status) {
  switch (status) {
  case FileStatusEnum::NOT_FOUND:
    return "File not found";
  default:
    return "Unknown file status";
  }
}
