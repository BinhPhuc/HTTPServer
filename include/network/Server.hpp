#pragma once

#include "network/ApiRouter.hpp"
#include "tls/TLS.hpp"
#include <netdb.h>
#include <thread_pool/thread_pool.hpp>

class Server {
private:
  int m_port;
  int m_sockfd;
  void stopServer();
  ApiRouter &m_api_router;
  ThreadPool thread_pool;
  SSL_CTX_ptr m_ctx;

public:
  Server(int port, ApiRouter &api_router);
  ~Server();
  void start();
};
