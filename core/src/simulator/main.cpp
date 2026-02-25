#include <fmt/base.h>

#include <App.h>

#include <crow.h>

int main()
{
  fmt::print("Hello, world!\n");

  uWS::App().get("/*", [](auto *res, auto *req) {
    res->end("Hello world!");
  }).listen(3000, [](auto *listen_socket) {
    if (listen_socket) {
      fmt::print("Listening on port {}\n", 3000);
    }
  }).run();

  fmt::print("Failed to listen on port 3000\n");
}

