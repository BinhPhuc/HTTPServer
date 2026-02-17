#pragma once

#include "network/ApiRouter.hpp"
#include <netdb.h>

class Server {
private:
  int m_port;
  int m_sockfd;
  void stopServer();
  ApiRouter &m_api_router;

public:
  Server(int port, ApiRouter &api_router);
  ~Server();
  void start();
};
