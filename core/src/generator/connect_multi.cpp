#include <crow.h>
#include "data_transfer.hpp"
#include "blocking_queue.hpp"
#include "stdlib.h"

#include "generator.hpp"

#include <map>
#include <unordered_set>
#include <chrono>

int main()
{
  crow::SimpleApp app;
  std::unordered_set<crow::websocket::connection *> users;
  std::mutex mtx;

  Generator global_gen(0.2, 0.3, 100, 150);
  std::map<
    crow::websocket::connection *,
    Blocking_Queue<double> *> data_buffers;

  std::map<
    crow::websocket::connection *,
    std::ofstream> file_handles;

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

      Blocking_Queue<double> *curr_buffer = new Blocking_Queue<double>();
      data_buffers.insert({&conn, curr_buffer});

      std::ofstream write_file(fmt::format("exports/{}.json", fmt::ptr(&conn)));
      write_file << "[\n";
      file_handles[&conn] = std::move(write_file);
    })
    .onclose([&](
      crow::websocket::connection &conn,
      const std::string &reason,
      uint16_t) {

      fmt::print("websocket connection closed: {}\n", reason);
      std::lock_guard<std::mutex> _(mtx);

      data_buffers.erase(&conn);

      users.erase(&conn);
    })
    .onmessage([&](
      crow::websocket::connection &conn,
      const std::string &data,
      bool is_binary) {

      std::lock_guard<std::mutex> _(mtx);

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

      if (data.starts_with("point"))
      {
        std::string point_substr = data.substr(6);
        double returned_val = std::stod(point_substr);
        Blocking_Queue<double> *conn_buffer = data_buffers.at(&conn);
        conn_buffer->enqueue(returned_val);
      }
    });

  auto server = app.port(5555).multithreaded().run_async();

  std::string line;
  while (std::getline(std::cin, line) && (line != "done")) {
    int pid = fork();

    if (pid == 0) {
      char *args[] = {
        (char *) "./scripting/.venv/bin/python3",
        (char *) line.c_str(),
        nullptr };
      execvp(args[0], args);
    }
  }

  fmt::print("global gen!\n");

  std::thread([&]{
    global_gen.generate_ws(&parameters, &users);
  }).detach();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::string print_str = "";
  for (auto const& [key, val] : data_buffers) {
    std::string formatted = fmt::format("CONN {} | ", fmt::ptr(key));
    print_str += formatted;
  }
  print_str.erase(print_str.length() - 3);
  // CONN 0x12f80f200
  fmt::print("{}\n", print_str);

  while (server.wait_for(std::chrono::seconds(0))
    == std::future_status::timeout) {

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::string print_str = "";
    for (auto const& [key, val] : data_buffers) {
      if (data_buffers.empty()) goto skip;

      double point = val->dequeue();
      std::string formatted = fmt::format("{:>16.4f} | ", point);
      print_str += formatted;

      file_handles.at(key) << "  " << point << ",\n";
    }
    print_str.erase(print_str.length() - 3);
    fmt::print("{}\n", print_str);

    skip: ;
  }

  server.wait();

  for (auto const& [key, val] : file_handles) {
    file_handles.at(key) << "  \"terminate\"\n";
    file_handles.at(key) << "]\n";
  }
}

