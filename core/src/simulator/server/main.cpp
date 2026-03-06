#include <fmt/core.h>
#include <fmt/format.h>

#include <stdint.h>
#include <unordered_set>

#include <crow.h>

struct Socket_Data
{
  uint64_t stock_point;
};

int main()
{
  crow::SimpleApp app;
  std::unordered_set<crow::websocket::connection *> users;
  std::unordered_map<crow::websocket::connection *, int> counts;
  std::mutex mtx;

  CROW_WEBSOCKET_ROUTE(app, "/")
    .onopen([&](crow::websocket::connection &conn) {
      fmt::print("new websocket connection from {}!\n", conn.get_remote_ip());
      std::lock_guard<std::mutex> _(mtx);
      users.insert(&conn);
      counts.insert({&conn, 0});
    })
    .onclose([&](
      crow::websocket::connection &conn,
      const std::string &reason,
      uint16_t) {

      fmt::print("websocket connection closed: {}\n", reason);
      std::lock_guard<std::mutex> _(mtx);
      users.erase(&conn);
      counts.erase(&conn);
    })
    .onmessage([&](
      crow::websocket::connection &conn,
      const std::string &data,
      bool is_binary) {
      
      std::lock_guard<std::mutex> _(mtx);
    });

  std::thread([&] {
    while (true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      std::lock_guard<std::mutex> _(mtx);
      for (auto *user : users)
      {
        user->send_text(fmt::format("{}", ++counts[user]));
      }
    }
  }).detach();

  app.port(1234).multithreaded().run();
}

