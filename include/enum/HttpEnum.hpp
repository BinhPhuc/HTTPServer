#pragma once
#include <string>

enum class HttpResponseStatusCodeEnum {
  OK,
  NOT_FOUND,
  BAD_REQUEST,
  INTERNAL_SERVER_ERROR,
  CONTENT_TOO_LARGE
};

enum class HttpResponseStatusMessageEnum {
  OK,
  NOT_FOUND,
  BAD_REQUEST,
  INTERNAL_SERVER_ERROR,
  CONTENT_TOO_LARGE
};

enum class HttpProtocolVersionEnum {
  HTTP_1_0,
  HTTP_1_1,
  HTTP_2_0,
};

enum class HttpMethodEnum { GET, POST, PUT, DELETE, PATCH, OPTIONS, HEAD };

std::string HttpResponseStatusMessage(HttpResponseStatusMessageEnum status);
std::string
HttpProtocolVersion(HttpProtocolVersionEnum version);
std::string HttpResponseStatusCode(HttpResponseStatusCodeEnum code);
std::string HttpMethod(HttpMethodEnum method);
