#include "enum/FileEnum.hpp"

std::string FileStatusMessage(FileStatusEnum status) {
  if (status == FileStatusEnum::NOT_FOUND) {
    return "File not found";
  } else if (status == FileStatusEnum::IS_DIRECTORY) {
    return "File is a directory";
  }
  return "Unknown file status";
}
