#pragma once

#include <string>

enum class StaticFileEnum {
  HTML,
  CSS,
  JS,
  PNG,
  JPG,
  GIF,
  SVG,
  ICO,
  JSON,
  XML,
  TXT,
  PDF,
  ZIP,
  OCTET_STREAM
};

std::string StaticFile(StaticFileEnum type);
