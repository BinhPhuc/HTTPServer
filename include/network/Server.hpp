#pragma once

#include "network/ApiRouter.hpp"
#include <netdb.h>
#include <thread_pool/thread_pool.hpp>

class Server {
private:
  int m_port;
  int m_sockfd;
  void stopServer();
  ApiRouter &m_api_router;
  ThreadPool thread_pool;

public:
  Server(int port, ApiRouter &api_router);
  ~Server();
  void start();
};
