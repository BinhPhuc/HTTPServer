#pragma once

#include <netdb.h>
class Server {
private:
  int m_port;
  int m_sockfd;
  void stopServer();

public:
  Server(int port);
  ~Server();
  void start();
};
