#include <fmt/core.h>
#include <fmt/format.h>

#include <stdint.h>
#include <unordered_set>

#include <crow.h>

struct Socket_Data
{
  uint64_t stock_point;
};

uint64_t data_points[] = {
  140, 74, 127, 341, 245, 148, 162, 128, 22, 463,
  446, 495, 331, 103, 34, 154, 331, 148, 304, 368,
  61, 326, 158, 263, 282, 214, 43, 479, 79, 484,
  341, 48, 129, 71, 20, 102, 220, 237, 268, 307,
  203, 67, 36, 220, 257, 41, 432, 99, 438, 483,
  462, 132, 235, 385, 321, 469, 473, 161, 290, 283,
  335, 334, 315, 264, 490, 113, 476, 122, 108, 376,
  293, 395, 274, 0, 388, 49, 184, 277, 32, 29,
  194, 110, 155, 249, 381, 379, 408, 116, 38, 373,
  168, 150, 434, 470, 149, 1, 367, 254, 32, 76,
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
        int curr = ++counts[user];
        fmt::print("sent: {}\n", data_points[curr]);
        user->send_text(fmt::format("{}", data_points[curr]));

        if (curr >= 100) goto loop_exit;
      }
    }

loop_exit:

  }).detach();

  app.port(1234).multithreaded().run();
}

