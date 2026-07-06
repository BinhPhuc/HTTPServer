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
  SSL_CTX_ptr m_ctx;
  ThreadPool thread_pool;
  bool m_keep_alive_enabled;

public:
  Server(int port, ApiRouter &api_router, size_t num_workers = 0,
         bool keep_alive_enabled = true);
  ~Server();
  void start();
};
