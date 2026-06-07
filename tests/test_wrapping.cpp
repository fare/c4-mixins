#include <c4/mixins.hpp>

#include <cassert>
#include <string>
#include <string_view>

struct log_base : c4::mixin {
  std::string log;
};

template<class Self, class Super>
struct HttpRequest : Super {
  void request(std::string_view path) {
    this->log += "HttpRequest(";
    this->log += path;
    this->log += ");";
  }
};

template<class Self, class Super>
struct Logging : Super {
  using c4_parents = c4::parents<HttpRequest>;

  void request(std::string_view path) {
    this->log += "Logging.before;";
    Super::request(path);
    this->log += "Logging.after;";
  }
};

template<class Self, class Super>
struct Timeout : Super {
  using c4_parents = c4::parents<HttpRequest>;

  void request(std::string_view path) {
    this->log += "Timeout.arm;";
    Super::request(path);
    this->log += "Timeout.disarm;";
  }
};

template<class Self, class Super>
struct Client : Super {
  using c4_parents = c4::parents<Logging, Timeout>;

  void request(std::string_view path) {
    this->log += "Client;";
    Super::request(path);
  }
};

using client_class = c4::instantiate<Client, log_base>;

int main() {
  client_class client;
  client.request("/status");

  assert(client.log ==
         "Client;"
         "Logging.before;"
         "Timeout.arm;"
         "HttpRequest(/status);"
         "Timeout.disarm;"
         "Logging.after;");
}
