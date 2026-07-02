#include "network/ApiRouter.hpp"
#include "enum/ContentTypeEnum.hpp"
#include "enum/FileEnum.hpp"
#include "enum/HttpEnum.hpp"
#include "handler/json/Json.hpp"
#include "handler/response/HttpResponseBuilder.hpp"
#include "http/HttpResponse.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string_view>
#include <sys/socket.h>
#include <utils/Helper.hpp>

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

  auto [static_response, found_static_file] =
      handle_get_static_file_request(request);

  if (!found_static_file) {
    auto [response, found_api] = handle_api_request(request);
    if (!found_api) {
      return HttpResponseBuilder::not_found();
    }
    return response;
  }

  return static_response;
}

std::pair<HttpResponse, bool>
ApiRouter::handle_api_request(const HttpRequest &request) {
  std::string key = request.get_method() + " " + request.get_path_only();
  auto it = m_routes.find(key);
  if (it != m_routes.end()) {
    return std::make_pair(it->second(request), true);
  } else {
    return std::make_pair(HttpResponseBuilder::not_found(), false);
  }
}

std::string from_file_to_byte(const std::filesystem::path &file_path) {
  if (std::filesystem::is_directory(file_path)) {
    spdlog::warn("File is a directory: {}, or maybe it is an api request",
                 file_path.string());
    return FileStatusMessage(FileStatusEnum::IS_DIRECTORY);
  }
  std::ifstream file(file_path, std::ios::binary);
  if (!file) {
    spdlog::warn("File not found: {}, maybe it is an api request",
                 file_path.string());
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
  std::filesystem::path result_path =
      std::filesystem::weakly_canonical(file_path);
  if (result_path.string().find(root_path.string() + "/" + m_root_folder +
                                "/") != 0) {
    spdlog::error("Static file request outside of root folder: {}",
                  result_path.string());
    res = HttpResponseBuilder::bad_request("Invalid file path");
    find_out = false;
    return std::make_pair(res, find_out);
  }
  std::string content = from_file_to_byte(result_path);
  if (content == FileStatusMessage(FileStatusEnum::NOT_FOUND)) {
    res = HttpResponseBuilder::not_found();
    find_out = false;
  } else if (content == FileStatusMessage(FileStatusEnum::IS_DIRECTORY)) {
    res = HttpResponseBuilder::bad_request("File is a directory");
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
  std::string content_type = request.get_header("Content-Type");
  std::string boundary = "";

  if (content_type.find("multipart/form-data") != std::string::npos) {
    boundary = "--" + content_type.substr(content_type.find("boundary=") + 9);
  }

  if (boundary.empty()) {
    return HttpResponseBuilder::bad_request(
        "Content-Type must be multipart/form-data with a boundary");
  }
  const std::string &body = request.get_body();
  std::string_view body_view(body);
  size_t search_pos = 0;
  while (true) {
    size_t start = body_view.find(boundary, search_pos);
    if (start == std::string_view::npos) {
      break;
    }
    size_t end = body_view.find(boundary, start + boundary.length());
    if (end == std::string_view::npos) {
      break;
    }
    std::string_view part = body_view.substr(start + boundary.length(),
                                             end - start - boundary.length());
    size_t filename_pos = part.find("filename=\"");
    if (filename_pos != std::string_view::npos) {
      size_t filename_start = filename_pos + 10;
      size_t filename_end = part.find("\"", filename_start);
      std::string filename(
          part.substr(filename_start, filename_end - filename_start));
      // nomarlize filename to avoid directory traversal attacks
      filename = std::filesystem::path(filename).filename().string();
      // random filename to avoid overwrite
      filename = std::to_string(std::time(nullptr)) + "_" +
                 utils::generate_random_number() + "_" + filename;
      size_t content_start = part.find("\r\n\r\n", filename_end);
      if (content_start != std::string_view::npos) {
        content_start += 4; // Skip the \r\n\r\n
        std::string_view file_content =
            part.substr(content_start, part.length() - content_start - 2);
        std::filesystem::path root_path =
            std::filesystem::path(PROJECT_ROOT_DIR);
        std::stringstream ss;
        ss << root_path.string() << "/" << m_root_folder << "/" << filename;
        std::string file_path = ss.str();
        std::ofstream outfile(file_path, std::ios::binary);
        if (!outfile) {
          spdlog::error("Failed to open file for writing: {}", file_path);
          return HttpResponseBuilder::internal_server_error(
              "Failed to save uploaded file");
        }
        outfile.write(file_content.data(),
                      static_cast<std::streamsize>(file_content.size()));
        outfile.close();
      }
    }
    search_pos = end;
  }
  std::string json_body =
      Json::json_body(HttpResponseStatusCode(HttpResponseStatusCodeEnum::OK),
                      "File uploaded successfully");
  HttpResponse res = HttpResponseBuilder::ok(json_body);
  res.set_header("Content-Type", "application/json; charset=utf-8");
  return res;
}

std::string ApiRouter::get_root_folder() const { return m_root_folder; }

void ApiRouter::set_root_folder(const std::string &root_folder) {
  m_root_folder = root_folder;
}
