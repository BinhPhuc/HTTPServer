#pragma once

#include "network/ApiRouter.hpp"
#include <netdb.h>
#include <thread_pool/thread_pool.hpp>

class Server {
private:
  int m_port;
  int m_sockfd;
  int m_epollfd;
  ApiRouter &m_api_router;

  void stop_server();
  bool initialize_socket();
  bool initialize_epoll();
  void run_event_loop();

public:
  Server(int port, ApiRouter &api_router);
  ~Server();
  void start();
};
