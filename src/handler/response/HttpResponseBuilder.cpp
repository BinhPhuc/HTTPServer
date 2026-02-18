#include "handler/response/HttpResponseBuilder.hpp"
#include "enum/HttpEnum.hpp"
#include "http/HttpResponse.hpp"

HttpResponse HttpResponseBuilder::ok(const std::string &body) {
  HttpResponse res;
  res.set_protocol_version(
      HttpResponseProtocolVersion(HttpResponseProtocolVersionEnum(
          HttpResponseProtocolVersionEnum::HTTP_1_1)));
  res.set_status_code(HttpResponseStatusCode(HttpResponseStatusCodeEnum::OK));
  res.set_status_message(
      HttpResponseStatusMessage(HttpResponseStatusMessageEnum::OK));
  res.set_status_line(res.get_protocol_version() + " " + res.get_status_code() +
                      " " + res.get_status_message());
  res.set_header("Content-Type", "application/json; charset=utf-8");
  res.set_body(body);
  res.set_header("Content-Length", std::to_string(res.get_body().size()));
  return res;
}

HttpResponse HttpResponseBuilder::not_found(const std::string &body) {
  HttpResponse res;
  res.set_protocol_version(
      HttpResponseProtocolVersion(HttpResponseProtocolVersionEnum(
          HttpResponseProtocolVersionEnum::HTTP_1_1)));
  res.set_status_code(
      HttpResponseStatusCode(HttpResponseStatusCodeEnum::NOT_FOUND));
  res.set_status_message(
      HttpResponseStatusMessage(HttpResponseStatusMessageEnum::NOT_FOUND));
  res.set_status_line(res.get_protocol_version() + " " + res.get_status_code() +
                      " " + res.get_status_message());
  res.set_header("Content-Type", "text/plain; charset=utf-8");
  res.set_body(body);
  res.set_header("Content-Length", std::to_string(res.get_body().size()));
  return res;
}

HttpResponse HttpResponseBuilder::bad_request(const std::string &body) {
  HttpResponse res;
  res.set_protocol_version(
      HttpResponseProtocolVersion(HttpResponseProtocolVersionEnum(
          HttpResponseProtocolVersionEnum::HTTP_1_1)));
  res.set_status_code(
      HttpResponseStatusCode(HttpResponseStatusCodeEnum::BAD_REQUEST));
  res.set_status_message(
      HttpResponseStatusMessage(HttpResponseStatusMessageEnum::BAD_REQUEST));
  res.set_status_line(res.get_protocol_version() + " " + res.get_status_code() +
                      " " + res.get_status_message());
  res.set_header("Content-Type", "text/plain; charset=utf-8");
  res.set_body(body);
  res.set_header("Content-Length", std::to_string(res.get_body().size()));
  return res;
}

std::string HttpResponseBuilder::build_response(const HttpResponse &response) {
  std::string res = response.get_status_line() + "\r\n";
  for (const auto &[key, value] : response.get_headers()) {
    res += key + ": " + value + "\r\n";
  }
  res += "\r\n";
  res += response.get_body();
  return res;
}
