#include "enum/FileEnum.hpp"

std::string FileStatusMessage(FileStatusEnum status) {
  if (status == FileStatusEnum::NOT_FOUND) {
    return "File not found";
  }
  return "Unknown file status";
}
