#include <crow.h>

#include "generator.hpp"

#include <unordered_set>

int main()
{
  crow::SimpleApp app;
  std::unordered_set<crow::websocket::connection *> users;
  std::mutex mtx;

  std::vector<Generator *> generators;

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

      if (data.starts_with("sim"))
      {
        std::string sim_args_str = data.substr(5);

        int find_pos = sim_args_str.find_first_of("!");
        double drift = std::stod(sim_args_str.substr(0, find_pos));

        sim_args_str = sim_args_str.substr(find_pos + 1);
        find_pos = sim_args_str.find_first_of("!");
        double volatility = std::stod(sim_args_str.substr(0, find_pos));

        sim_args_str = sim_args_str.substr(find_pos + 1);
        find_pos = sim_args_str.find_first_of("!");
        int price = std::stoi(sim_args_str.substr(0, find_pos));

        sim_args_str = sim_args_str.substr(find_pos + 1);
        int target = std::stoi(sim_args_str);

        Generator *new_generator =
          new Generator(drift, volatility, price, target);
        generators.push_back(new_generator);

        fmt::print("[id: {}]: {} {} {} {}\n",
          generators.size(),
          drift,
          volatility,
          price,
          target
        );

        std::thread([&]{
          new_generator->generate_ws(&parameters, generators.size());
        }).detach();
      }

      if (data.starts_with("pause"))
      {
        parameters.pause.store(true);
      }

      if (data.starts_with("resume"))
      {
        parameters.pause.store(false);
      }

      if (data.starts_with("stop"))
      {
        parameters.gen.store(false);
      }
    });

  app.port(5555).multithreaded().run();

  parameters.gen.store(false);
}

