#pragma once

#include <model/User.hpp>
#include <network/ApiRouter.hpp>
#include <vector>

class UserController {
public:
  void registerRoutes(ApiRouter &r);

private:
  std::vector<User> listUsers(const HttpRequest &);
  User me(const HttpRequest &);
};
