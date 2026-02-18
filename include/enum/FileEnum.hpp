#pragma once

#include <string>

enum class FileStatusEnum {
  NOT_FOUND,
};

std::string FileStatusMessage(FileStatusEnum status);
