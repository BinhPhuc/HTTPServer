#pragma once

#include <string>

enum class FileStatusEnum {
  NOT_FOUND,
  IS_DIRECTORY,
};

std::string FileStatusMessage(FileStatusEnum status);
