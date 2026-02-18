#pragma once
#include <string>

enum class HttpResponseStatusCodeEnum { OK, NOT_FOUND, BAD_REQUEST };

enum class HttpResponseStatusMessageEnum { OK, NOT_FOUND, BAD_REQUEST };

enum class HttpResponseProtocolVersionEnum {
  HTTP_1_0,
  HTTP_1_1,
  HTTP_2_0,
};

std::string HttpResponseStatusMessage(HttpResponseStatusMessageEnum status);
std::string
HttpResponseProtocolVersion(HttpResponseProtocolVersionEnum version);
std::string HttpResponseStatusCode(HttpResponseStatusCodeEnum code);
