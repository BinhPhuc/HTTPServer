#include "enum/FileEnum.hpp"

std::string FileStatusMessage(FileStatusEnum status) {
  switch (status) {
  case FileStatusEnum::NOT_FOUND:
    return "File not found";
  case FileStatusEnum::IS_DIRECTORY:
    return "File is a directory";
  default:
    return "Unknown file status";
  }
}
