#include "enum/ContentTypeEnum.hpp"

std::string StaticFile(StaticFileEnum type) {
  switch (type) {
  case StaticFileEnum::HTML:
    return "text/html";
  case StaticFileEnum::CSS:
    return "text/css";
  case StaticFileEnum::JS:
    return "application/javascript";
  case StaticFileEnum::PNG:
    return "image/png";
  case StaticFileEnum::JPG:
    return "image/jpeg";
  case StaticFileEnum::GIF:
    return "image/gif";
  case StaticFileEnum::SVG:
    return "image/svg+xml";
  case StaticFileEnum::ICO:
    return "image/x-icon";
  case StaticFileEnum::JSON:
    return "application/json";
  case StaticFileEnum::XML:
    return "application/xml";
  case StaticFileEnum::TXT:
    return "text/plain";
  case StaticFileEnum::PDF:
    return "application/pdf";
  case StaticFileEnum::ZIP:
    return "application/zip";
  default:
    return "application/octet-stream";
  }
}
