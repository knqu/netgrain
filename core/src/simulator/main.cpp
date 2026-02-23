#include <stdio.h>

#include <fmt/base.h>

#include <crow.h>

int main()
{
  fmt::println("Hello, world!");

  crow::SimpleApp app;

  CROW_ROUTE(app, "/")([]() {
    return "Hello, world!";
  });

  app.port(8080).multithreaded().run();
}

