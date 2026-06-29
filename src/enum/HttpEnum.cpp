#include "enum/HttpEnum.hpp"

std::string HttpResponseStatusMessage(HttpResponseStatusMessageEnum status) {
  switch (status) {
  case HttpResponseStatusMessageEnum::OK:
    return "OK";
  case HttpResponseStatusMessageEnum::NOT_FOUND:
    return "Not Found";
  case HttpResponseStatusMessageEnum::BAD_REQUEST:
    return "Bad Request";
  case HttpResponseStatusMessageEnum::INTERNAL_SERVER_ERROR:
    return "Internal Server Error";
  default:
    return "Unknown Status";
  }
}

std::string
HttpResponseProtocolVersion(HttpResponseProtocolVersionEnum version) {
  switch (version) {
  case HttpResponseProtocolVersionEnum::HTTP_1_0:
    return "HTTP/1.0";
  case HttpResponseProtocolVersionEnum::HTTP_1_1:
    return "HTTP/1.1";
  case HttpResponseProtocolVersionEnum::HTTP_2_0:
    return "HTTP/2.0";
  default:
    return "Unknown Protocol Version";
  }
}

std::string HttpMethod(HttpMethodEnum method) {
  switch (method) {
  case HttpMethodEnum::GET:
    return "GET";
  case HttpMethodEnum::POST:
    return "POST";
  case HttpMethodEnum::PUT:
    return "PUT";
  case HttpMethodEnum::DELETE:
    return "DELETE";
  case HttpMethodEnum::PATCH:
    return "PATCH";
  case HttpMethodEnum::OPTIONS:
    return "OPTIONS";
  case HttpMethodEnum::HEAD:
    return "HEAD";
  default:
    return "Unknown Method";
  }
}

std::string HttpResponseStatusCode(HttpResponseStatusCodeEnum code) {
  switch (code) {
  case HttpResponseStatusCodeEnum::OK:
    return "200";
  case HttpResponseStatusCodeEnum::NOT_FOUND:
    return "404";
  case HttpResponseStatusCodeEnum::BAD_REQUEST:
    return "400";
  case HttpResponseStatusCodeEnum::INTERNAL_SERVER_ERROR:
    return "500";
  default:
    return "Unknown Status Code";
  }
}
