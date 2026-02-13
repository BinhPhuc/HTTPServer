#include <nlohmann/json.hpp>
#include <string>

class User {
public:
  User(int id, const std::string &name, const std::string &email);
  // ~User();
  void set_id(int new_id);
  int get_id() const;
  void set_name(const std::string &new_name);
  std::string get_name() const;
  void set_email(const std::string &new_email);
  std::string get_email() const;

  int id;
  std::string name;
  std::string email;
};

inline void to_json(nlohmann::json &j, const User &u) {
  j = nlohmann::json{{"id", u.id}, {"name", u.name}, {"email", u.email}};
}

inline void from_json(const nlohmann::json &j, User &u) {
  j.at("id").get_to(u.id);
  j.at("name").get_to(u.name);
  j.at("email").get_to(u.email);
}
