#include "network/ApiRouter.hpp"
#include "enum/ContentTypeEnum.hpp"
#include "enum/FileEnum.hpp"
#include "enum/HttpEnum.hpp"
#include "handler/response/HttpResponseBuilder.hpp"
#include "http/HttpResponse.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <sys/socket.h>

using json = nlohmann::json;

ApiRouter::ApiRouter() : m_routes(), m_root_folder() {}

void ApiRouter::add(const std::string &method, const std::string &path,
                    Handler handler) {
  std::string key = method + " " + path;
  m_routes[key] = handler;
}

HttpResponse ApiRouter::dispatch(const HttpRequest &request) {
  std::string resource_path = request.get_path_only();
  if (request.get_method() == HttpMethod(HttpMethodEnum::POST) &&
      resource_path == "/upload") {
    return handle_upload_static_file_request(request);
  }
  if (request.get_method() == HttpMethod(HttpMethodEnum::POST) &&
      resource_path != "/upload") {
    return HttpResponseBuilder::bad_request("You must upload at route /upload");
  }
  std::pair<HttpResponse, bool> static_response =
      handle_get_static_file_request(request);
  if (static_response.second) {
    return static_response.first;
  }
  return handle_api_request(request);
}

HttpResponse ApiRouter::handle_api_request(const HttpRequest &request) {
  spdlog::info("chay vao day");
  std::string key = request.get_method() + " " + request.get_path_only();
  auto it = m_routes.find(key);
  if (it != m_routes.end()) {
    return it->second(request);
  } else {
    return HttpResponseBuilder::not_found();
  }
}

std::string from_file_to_byte(const std::string &file_path) {
  std::ifstream file(file_path, std::ios::binary);
  if (!file) {
    spdlog::error("File not found: {}", file_path);
    return FileStatusMessage(FileStatusEnum::NOT_FOUND);
  }
  std::string content((std::istreambuf_iterator<char>(file)),
                      std::istreambuf_iterator<char>());
  return content;
}

std::string get_content_type(const std::string &file_path) {
  std::string extension = file_path.substr(file_path.find_last_of('.') + 1);
  if (extension == "html") {
    return StaticFile(StaticFileEnum::HTML);
  } else if (extension == "css") {
    return StaticFile(StaticFileEnum::CSS);
  } else if (extension == "js") {
    return StaticFile(StaticFileEnum::JS);
  } else if (extension == "png") {
    return StaticFile(StaticFileEnum::PNG);
  } else if (extension == "jpg" || extension == "jpeg") {
    return StaticFile(StaticFileEnum::JPG);
  } else if (extension == "gif") {
    return StaticFile(StaticFileEnum::GIF);
  } else if (extension == "svg") {
    return StaticFile(StaticFileEnum::SVG);
  } else if (extension == "ico") {
    return StaticFile(StaticFileEnum::ICO);
  } else if (extension == "json") {
    return StaticFile(StaticFileEnum::JSON);
  } else if (extension == "xml") {
    return StaticFile(StaticFileEnum::XML);
  } else if (extension == "txt") {
    return StaticFile(StaticFileEnum::TXT);
  } else if (extension == "pdf") {
    return StaticFile(StaticFileEnum::PDF);
  } else if (extension == "zip") {
    return StaticFile(StaticFileEnum::ZIP);
  } else {
    return StaticFile(StaticFileEnum::OCTET_STREAM); // default to octet-stream
  }
}

std::pair<HttpResponse, bool>
ApiRouter::handle_get_static_file_request(const HttpRequest &request) {
  HttpResponse res;
  bool find_out = true;
  if (request.get_method() != HttpMethod(HttpMethodEnum::GET)) {
    find_out = false;
    return std::make_pair(
        HttpResponseBuilder::bad_request("Method not allowed"), find_out);
  }
  std::string resource_path = request.get_path_only();
  std::filesystem::path root_path = std::filesystem::path(PROJECT_ROOT_DIR);
  std::stringstream ss;
  ss << root_path.string() << "/" << m_root_folder << resource_path;
  std::string file_path = ss.str();
  std::string content = from_file_to_byte(file_path);
  if (content == FileStatusMessage(FileStatusEnum::NOT_FOUND)) {
    res = HttpResponseBuilder::not_found();
    find_out = false;
  } else {
    res = HttpResponseBuilder::ok(content);
    find_out = true;
    std::string content_type = get_content_type(file_path);
    res.set_header("Content-Type", content_type);
  }
  return std::make_pair(res, find_out);
}

HttpResponse
ApiRouter::handle_upload_static_file_request(const HttpRequest &request) {
  std::vector<std::string> content_types = request.get_headers("Content-Type");
  std::string boundary = "";
  for (const std::string &content_type : content_types) {
    if (content_type.find("multipart/form-data") != std::string::npos) {
      boundary = "--" + content_type.substr(content_type.find("boundary=") + 9);
      break;
    }
  }
  if (boundary.empty()) {
    return HttpResponseBuilder::bad_request(
        "Content-Type must be multipart/form-data with a boundary");
  }
  std::string body = request.get_body();
  while (body.find(boundary) != std::string::npos) {
    size_t start = body.find(boundary);
    size_t end = body.find(boundary, start + boundary.length());
    if (end == std::string::npos) {
      break;
    }
    std::string part =
        body.substr(start + boundary.length(), end - start - boundary.length());
    size_t filename_pos = part.find("filename=\"");
    if (filename_pos != std::string::npos) {
      size_t filename_start = filename_pos + 10;
      size_t filename_end = part.find("\"", filename_start);
      std::string filename =
          part.substr(filename_start, filename_end - filename_start);
      // random filename to avoid overwrite
      filename = std::to_string(std::time(nullptr)) + "_" + filename;
      size_t content_start = part.find("\r\n\r\n", filename_end);
      if (content_start != std::string::npos) {
        content_start += 4; // Skip the \r\n\r\n
        std::string file_content =
            part.substr(content_start, part.length() - content_start - 2);
        std::filesystem::path root_path =
            std::filesystem::path(PROJECT_ROOT_DIR);
        std::stringstream ss;
        ss << root_path.string() << "/" << m_root_folder << "/" << filename;
        std::string file_path = ss.str();
        std::ofstream outfile(file_path, std::ios::binary);
        outfile.write(file_content.c_str(), file_content.size());
        outfile.close();
      }
    }
    body.erase(0, end);
  }
  HttpResponse res = HttpResponseBuilder::ok("File uploaded successfully");
  res.set_header("Content-Type", "application/json, charset=utf-8");
  json response_body = {{"message", "File uploaded successfully"}};
  res.set_body(response_body.dump());
  return res;
}

std::string ApiRouter::get_root_folder() const { return m_root_folder; }

void ApiRouter::set_root_folder(const std::string &root_folder) {
  m_root_folder = root_folder;
}
