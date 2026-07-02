#include "handler/response/HttpResponseBuilder.hpp"
#include "enum/HttpEnum.hpp"
#include "handler/json/Json.hpp"
#include "http/HttpResponse.hpp"
#include <utils/Helper.hpp>

const std::string HTML_404_PAGE = R"html(
<!DOCTYPE html>
<html lang="vi">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>404 - Not Found </title>
    <style>
        * {
            box-sizing: border-box;
        }
        body {
            margin: 0;
            padding: 0;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #f4f6f9;
            color: #333;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            text-align: center;
        }
        .error-container {
            max-width: 500px;
            padding: 40px;
            background: #ffffff;
            border-radius: 10px;
            box-shadow: 0 10px 25px rgba(0, 0, 0, 0.05);
            margin: 20px;
        }
        h1 {
            font-size: 100px;
            margin: 0;
            color: #ff6b6b;
            line-height: 1;
        }
        h2 {
            font-size: 24px;
            margin: 15px 0;
            color: #2c3e50;
        }
        p {
            color: #7f8c8d;
            margin-bottom: 30px;
            line-height: 1.6;
        }
        .btn-home {
            display: inline-block;
            text-decoration: none;
            background-color: #3498db;
            color: white;
            padding: 12px 25px;
            border-radius: 5px;
            font-weight: 600;
            transition: background-color 0.3s ease, transform 0.2s ease;
        }
        .btn-home:hover {
            background-color: #2980b9;
            transform: translateY(-2px);
        }
    </style>
</head>
<body>
    <div class="error-container">
        <h1>404</h1>
        <h2>Resource Not Found</h2>
        <p>Sorry your request resource not found. Please check the URL.</p>
    </div>
</body>
</html>
)html";

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
  res.set_header("Date", utils::rfc_date());
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
  res.set_header("Date", utils::rfc_date());
  return res;
}

HttpResponse HttpResponseBuilder::not_found() {
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
  res.set_header("Content-Type", "text/html; charset=utf-8");
  std::string body = HTML_404_PAGE;
  res.set_body(body);
  res.set_header("Content-Length", std::to_string(res.get_body().size()));
  res.set_header("Date", utils::rfc_date());
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
  res.set_header("Content-Type", "application/json; charset=utf-8");
  res.set_body(Json::json_body(
      HttpResponseStatusCode(HttpResponseStatusCodeEnum::BAD_REQUEST), body));
  res.set_header("Content-Length", std::to_string(res.get_body().size()));
  res.set_header("Date", utils::rfc_date());
  return res;
}

HttpResponse
HttpResponseBuilder::internal_server_error(const std::string &body) {
  HttpResponse res;
  res.set_protocol_version(
      HttpResponseProtocolVersion(HttpResponseProtocolVersionEnum(
          HttpResponseProtocolVersionEnum::HTTP_1_1)));
  res.set_status_code(HttpResponseStatusCode(
      HttpResponseStatusCodeEnum::INTERNAL_SERVER_ERROR));
  res.set_status_message(HttpResponseStatusMessage(
      HttpResponseStatusMessageEnum::INTERNAL_SERVER_ERROR));
  res.set_status_line(res.get_protocol_version() + " " + res.get_status_code() +
                      " " + res.get_status_message());
  res.set_header("Content-Type", "application/json; charset=utf-8");
  res.set_body(Json::json_body(res.get_status_code(), body));
  res.set_header("Content-Length", std::to_string(res.get_body().size()));
  res.set_header("Date", utils::rfc_date());
  return res;
}

HttpResponse HttpResponseBuilder::content_too_large(const std::string &body) {
  HttpResponse res;
  res.set_protocol_version(
      HttpResponseProtocolVersion(HttpResponseProtocolVersionEnum(
          HttpResponseProtocolVersionEnum::HTTP_1_1)));
  res.set_status_code(
      HttpResponseStatusCode(HttpResponseStatusCodeEnum::CONTENT_TOO_LARGE));
  res.set_status_message(HttpResponseStatusMessage(
      HttpResponseStatusMessageEnum::CONTENT_TOO_LARGE));
  res.set_status_line(res.get_protocol_version() + " " + res.get_status_code() +
                      " " + res.get_status_message());
  res.set_header("Content-Type", "application/json; charset=utf-8");
  res.set_body(Json::json_body(res.get_status_code(), body));
  res.set_header("Content-Length", std::to_string(res.get_body().size()));
  res.set_header("Date", utils::rfc_date());
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
