#include <crow.h>

#include "generator.hpp"

#include <unordered_set>

int main()
{
  crow::SimpleApp app;
  std::unordered_set<crow::websocket::connection *> users;
  std::mutex mtx;

  Generator global_gen(0.2, 0.3, 100, 150);
  Data_Transfer parameters;
  parameters.conn.store(nullptr);
  parameters.gen.store(true);
  parameters.new_event.store(0);
  parameters.send_data.store(false);

  CROW_WEBSOCKET_ROUTE(app, "/")
    .onopen([&](crow::websocket::connection &conn) {
      fmt::print("new websocket connection from {}!\n", conn.get_remote_ip());
      std::lock_guard<std::mutex> _(mtx);
      users.insert(&conn);
      parameters.conn.store(&conn);
      parameters.send_data.store(true);
    })
    .onclose([&](
      crow::websocket::connection &conn,
      const std::string &reason,
      uint16_t) {

      fmt::print("websocket connection closed: {}\n", reason);
      std::lock_guard<std::mutex> _(mtx);
      parameters.send_data.store(false);
      parameters.conn.store(nullptr);
      users.erase(&conn);
    })
    .onmessage([&](
      crow::websocket::connection &conn,
      const std::string &data,
      bool is_binary) {

      std::lock_guard<std::mutex> _(mtx);
      if (data == "flash_crash")
      {
        if (parameters.new_event.load() == 0)
        {
          parameters.new_event.store(1);
          fmt::print("flash crash!\n");
        }
      }

      if (data == "sideways") { // HX
        if (parameters.new_event.load() == 0) {
          parameters.new_event.store(3);
          fmt::print("sideways!\n");
        }
      }

      if (data == "stop")
      {
        parameters.send_data.store(false);
      }

      if (data.starts_with("bubble"))
      {
        if (parameters.new_event.load() == 0)
        {
          int threshold = std::stoi(data.substr(7), nullptr, 10);
          parameters.new_event.store(2);
          parameters.threshold.store(threshold);
          fmt::print("bubble! {}\n", threshold);
        }
        else if (parameters.new_event.load() == 2)
        {
          fmt::print("bubble is ignored: called consecutively when another is active!\n");
        }
      }
    });

  std::thread([&]{
    global_gen.generate_ws(&parameters);
  }).detach();

  app.port(5555).multithreaded().run();

  parameters.gen.store(false);
}

