#pragma once

#include "network/ApiRouter.hpp"
#include "tls/TLS.hpp"
#include <cstddef>
#include <thread>
#include <vector>

class Server {
private:
  int m_port;
  size_t m_num_loops;
  ApiRouter &m_api_router;
  SSL_CTX_ptr m_ssl_ctx;
  std::vector<std::thread> m_workers;

  int create_listen_socket();
  void run_event_loop(int loop_id, int wakeup_fd);

public:
  Server(int port, ApiRouter &api_router,
         size_t num_loops = std::thread::hardware_concurrency());
  ~Server();
  void start();
};
