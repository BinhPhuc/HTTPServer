#include "handler/json/Json.hpp"
#include <controller/UserController.hpp>
#include <handler/response/HttpResponseBuilder.hpp>
#include <vector>

void UserController::registerRoutes(ApiRouter &r) {
  r.add("GET", "/api/users", [this](const HttpRequest &req) {
    std::vector<User> users = this->listUsers(req);
    return HttpResponseBuilder::ok(Json::stringify(users));
  });
  // r.add("GET", "/api/users/me",
  //       [this](const HttpRequest &req) { return this->me(req); });
}

std::vector<User> UserController::listUsers(const HttpRequest &) {
  return {{1, "binhphuc", "binhphuc@gmail.com"},
          {2, "binhpp", "binhpp@falcongames.com"}};
}
