// 02_wrapping.cpp
//
// Cooperative before/after method wrapping.
//
// Each mixin can run code before and after Super::request(). The linearization
// determines the next method in the chain.

#include <c4/mixins.hpp>

#include <iostream>
#include <string_view>

template<class Self, class Super>
struct HttpRequest : Super {
  void request(std::string_view path) {
    std::cout << "HttpRequest: GET " << path << "\n";
  }
};

template<class Self, class Super>
struct Logging : Super {
  using c4_parents = c4::parents<HttpRequest>;

  void request(std::string_view path) {
    std::cout << "Logging: before\n";
    Super::request(path);
    std::cout << "Logging: after\n";
  }
};

template<class Self, class Super>
struct Timeout : Super {
  using c4_parents = c4::parents<HttpRequest>;

  void request(std::string_view path) {
    std::cout << "Timeout: arm\n";
    Super::request(path);
    std::cout << "Timeout: disarm\n";
  }
};

template<class Self, class Super>
struct Client : Super {
  using c4_parents = c4::parents<Logging, Timeout>;

  void request(std::string_view path) {
    std::cout << "Client: request\n";
    Super::request(path);
  }
};

using MyClient = c4::instantiate<Client>;

int main() {
  MyClient client;
  client.request("/status");

  // Output:
  //
  // Client: request
  // Logging: before
  // Timeout: arm
  // HttpRequest: GET /status
  // Timeout: disarm
  // Logging: after
}
