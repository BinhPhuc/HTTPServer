#include "network/ApiRouter.hpp"
#include "enum/ContentTypeEnum.hpp"
#include "enum/FileEnum.hpp"
#include "handler/response/HttpResponseBuilder.hpp"
#include "http/HttpResponse.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <vector>

ApiRouter::ApiRouter() : m_routes(), m_root_folder() {}

void ApiRouter::add(const std::string &method, const std::string &path,
                    Handler handler) {
  std::string key = method + " " + path;
  m_routes[key] = handler;
}

HttpResponse ApiRouter::dispatch(const HttpRequest &request) {
  std::vector<std::string> segments = split_path(request.get_path_only());
  std::vector<std::string> static_files;
  for (const auto &segment : segments) {
    if (segment.find('.') != std::string::npos) {
      static_files.push_back(segment);
    }
  }
  if (!static_files.empty()) {
    return handle_static_file_request(request, static_files);
  } else {
    return handle_api_request(request);
  }
}

std::vector<std::string> ApiRouter::split_path(const std::string &path) const {
  std::vector<std::string> segments;
  size_t start = 0, end = 0;
  while ((end = path.find('/', start)) != std::string::npos) {
    if (end != start) {
      segments.push_back(path.substr(start, end - start));
    }
    start = end + 1;
  }
  if (start < path.size()) {
    segments.push_back(path.substr(start));
  }
  return segments;
}

HttpResponse ApiRouter::handle_api_request(const HttpRequest &request) {
  std::string key = request.get_method() + " " + request.get_path_only();
  auto it = m_routes.find(key);
  if (it != m_routes.end()) {
    return it->second(request);
  } else {
    return HttpResponseBuilder::not_found("Not Found");
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
    return StaticFile(StaticFileEnum::TXT); // default to text/plain
  }
}

HttpResponse ApiRouter::handle_static_file_request(
    const HttpRequest &request, const std::vector<std::string> &static_files) {
  if (request.get_method() != "GET") {
    return HttpResponseBuilder::bad_request("Method not allowed");
  }
  if (static_files.size() > 1) {
    return HttpResponseBuilder::bad_request("Invalid path");
  }
  std::filesystem::path root_path = std::filesystem::path(PROJECT_ROOT_DIR);
  std::string file_path = (root_path / m_root_folder / static_files[0]).string();
  spdlog::info("Serving static file: {}", file_path);
  HttpResponse res;
  std::string content = from_file_to_byte(file_path);
  if (content == FileStatusMessage(FileStatusEnum::NOT_FOUND)) {
    res = HttpResponseBuilder::not_found(content);
  } else {
    res = HttpResponseBuilder::ok(content);
    std::string content_type = get_content_type(file_path);
    res.set_header("Content-Type", content_type);
  }
  return res;
}

std::string ApiRouter::get_root_folder() const { return m_root_folder; }

void ApiRouter::set_root_folder(const std::string &root_folder) {
  m_root_folder = root_folder;
}
