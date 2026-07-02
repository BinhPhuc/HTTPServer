#include "enum/ConnectionState.hpp"
#include "enum/HttpEnum.hpp"
#include "handler/request/HttpRequestParser.hpp"
#include "handler/request/HttpsRequestReader.hpp"
#include "handler/response/HttpResponseBuilder.hpp"
#include "handler/response/HttpsResponseSender.hpp"
#include "handler/shutdown/shutdown.hpp"
#include "http/HttpResponse.hpp"
#include "network/ApiRouter.hpp"
#include "tls/TLS.hpp"
#include "utils/Constants.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <handler/json/Json.hpp>
#include <model/User.hpp>
#include <netdb.h>
#include <network/Server.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <utils/Helper.hpp>

Server::Server(int port, ApiRouter &api_router)
    : m_port(port), m_sockfd(-1), m_api_router(api_router),
      m_ctx(TLS::create_context()), thread_pool() {
  TLS::configure_context(m_ctx.get());
}

Server::~Server() { stopServer(); }

void Server::stopServer() {
  if (m_sockfd != -1) {
    spdlog::info("Stopping server on port {}.", m_port);

    close(m_sockfd);
    m_sockfd = -1;
  }
  spdlog::info("Server stopped.");
}

int create_socket(int domain, int type, int protocol) {
  int sockfd = socket(domain, type, protocol);
  if (sockfd == -1) {
    spdlog::error("Socket creation error: {}", strerror(errno));
  }
  return sockfd;
}

void get_addr_info_wrapper(const char *node, const char *service,
                           const struct addrinfo *hints,
                           struct addrinfo **res) {
  int status = getaddrinfo(node, service, hints, res);
  if (status != 0) {
    spdlog::error("getaddrinfo error: {}", gai_strerror(status));
  }
}

int set_sock_option(int sockfd, int level, int optname) {
  int yes = 1;
  int status = setsockopt(sockfd, level, optname, &yes, sizeof(int));
  if (status == -1) {
    spdlog::error("Setsockopt error: {}", strerror(errno));
  }
  return status;
}

int bind_socket(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
  int status = bind(sockfd, addr, addrlen);
  if (status == -1) {
    spdlog::error("Bind error: {}", strerror(errno));
  }
  return status;
}

int socket_listen(int sockfd, int backlog) {
  int status = listen(sockfd, backlog);
  if (status == -1) {
    spdlog::error("Listen error: {}", strerror(errno));
  }
  return status;
}

void Server::start() {
  // getaddrinfo -> get socket fd -> bind -> listen -> accept
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  get_addr_info_wrapper(NULL, std::to_string(m_port).c_str(), &hints, &res);

  struct addrinfo *p;
  for (p = res; p != NULL; p = p->ai_next) {
    m_sockfd = create_socket(p->ai_family, p->ai_socktype, p->ai_protocol);

    if (set_sock_option(m_sockfd, SOL_SOCKET, SO_REUSEADDR) == -1) {
      close(m_sockfd);
      continue;
    }

    if (bind_socket(m_sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(m_sockfd);
      continue;
    }

    break;
  }

  freeaddrinfo(res);

  if (p == NULL) {
    spdlog::error("Failed to bind socket.");
    return;
  }

  if (socket_listen(m_sockfd, config::MAX_CONNECTIONS) == -1) {
    close(m_sockfd);
    return;
  }

  spdlog::info("Server started on port {}.", m_port);

  ShutdownHandler::listen_fd.store(m_sockfd);

  struct sockaddr_storage their_addr;

  while (ShutdownHandler::running.load()) {
    socklen_t addr_size = sizeof(their_addr);
    int new_fd = accept(m_sockfd, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == -1) {
      if (!ShutdownHandler::running.load()) {
        break;
      }
      spdlog::error("Accept error: {}", strerror(errno));
      continue;
    }

    thread_pool.enqueue([this, new_fd]() {
      try {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        spdlog::info("Thread {} handling connection {}", oss.str(), new_fd);

        SSL_ptr ssl(SSL_new(m_ctx.get()));

        TLS::set_fd(ssl.get(), new_fd);

        int ret = TLS::accept(ssl.get());

        if (ret <= 0) {
          spdlog::error("TLS handshake failed for connection {}", new_fd);
          ssl.reset();
          close(new_fd);
          return;
        }

        struct timeval timeout;

        timeout.tv_sec = config::SOCKET_TIMEOUT;
        timeout.tv_usec = 0;

        if (setsockopt(new_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout,
                       sizeof(timeout)) < 0) {
          spdlog::error("setsockopt failed: {}", strerror(errno));
          ShutdownHandler::close_connection(ssl.get(), new_fd, true);
          return;
        }

        auto handle_error_from_request =
            [new_fd, ssl = ssl.get()](const std::string &error_message,
                                      const HttpResponse &response) {
              spdlog::error("{} from connection {}", error_message, new_fd);
              std::string response_str =
                  HttpResponseBuilder::build_response(response);
              if (HttpsResponseSender::send_all(ssl, response_str.c_str(),
                                                response_str.size()) == -1) {
                spdlog::error("Send error: {}", strerror(errno));
              }
              ShutdownHandler::close_connection(ssl, new_fd, true);
            };

        bool keep_alive = true;
        HttpsRequestReader request_reader(ssl.get());

        while (keep_alive && ShutdownHandler::running.load()) {
          std::string raw_request = request_reader.read_request();

          if (raw_request == ConnectionState(ConnectionStateEnum::ERROR)) {
            HttpResponse response = HttpResponseBuilder::internal_server_error(
                "Error reading request");
            response.set_header("Connection", "close");
            handle_error_from_request("Error reading request", response);
            return;
          }

          if (raw_request == ConnectionState(ConnectionStateEnum::TOO_LARGE)) {
            HttpResponse response = HttpResponseBuilder::content_too_large(
                "Exceeded maximum upload size (10MB)");
            response.set_header("Connection", "close");
            handle_error_from_request("Exceeded maximum upload size (10MB)",
                                      response);
            return;
          }

          if (raw_request == ConnectionState(ConnectionStateEnum::BAD)) {
            HttpResponse response =
                HttpResponseBuilder::bad_request("Bad request");
            response.set_header("Connection", "close");
            handle_error_from_request("Bad request", response);
            return;
          }

          if (raw_request == ConnectionState(ConnectionStateEnum::CLOSED) ||
              raw_request == ConnectionState(ConnectionStateEnum::TIMEOUT)) {
            spdlog::info("Connection {} closed by client or timed out", new_fd);
            break;
          }

          auto [request, is_valid_request] =
              HttpRequestParser::parse(raw_request);

          if (!is_valid_request) {
            HttpResponse response =
                HttpResponseBuilder::bad_request("Bad request");
            response.set_header("Connection", "close");
            handle_error_from_request("Bad request", response);
            return;
          }

          std::string connection_header = request.get_header("Connection");
          std::string http_version = request.get_version();

          keep_alive = (http_version ==
                        HttpProtocolVersion(HttpProtocolVersionEnum::HTTP_1_1));

          if (connection_header.empty()) {
            connection_header = keep_alive ? "keep-alive" : "close";
          } else {
            connection_header = utils::lowercase(connection_header);
            if (connection_header == "keep-alive") {
              keep_alive = true;
            } else if (connection_header == "close") {
              keep_alive = false;
            }
          }

          oss.str("");
          oss << std::this_thread::get_id();
          spdlog::info("Thread {} - {} {} {}", oss.str(), request.get_method(),
                       request.get_path(), request.get_version());

          HttpResponse response = m_api_router.dispatch(request);
          response.set_header("Connection", connection_header);
          std::string response_str =
              HttpResponseBuilder::build_response(response);

          if (HttpsResponseSender::send_all(ssl.get(), response_str.c_str(),
                                            response_str.size()) == -1) {
            spdlog::error("Send error: {}", strerror(errno));
          }
        }
        ShutdownHandler::close_connection(ssl.get(), new_fd, false);
      } catch (const std::exception &e) {
        spdlog::error("Exception in connection handler: {}", e.what());
        close(new_fd);
      }
    });
  }
}
