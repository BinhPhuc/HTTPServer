#pragma once

#include "network/ApiRouter.hpp"
#include <netdb.h>
#include <thread_pool/thread_pool.hpp>

class Server {
private:
  int m_port;
  int m_sockfd;
  int m_epollfd;
  void stopServer();
  ApiRouter &m_api_router;
  ThreadPool thread_pool;
  struct ConnectionState {
    std::string read_buffer;
    std::string write_buffer;
    int fd;
    size_t write_offset;
    enum state { READING, WRITING, PROCESSING };
  };

public:
  Server(int port, ApiRouter &api_router);
  ~Server();
  void start();
};
