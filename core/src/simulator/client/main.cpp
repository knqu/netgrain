#include <iostream>

#include <fmt/core.h>

#include <pybind11/embed.h>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXUserAgent.h>

#include "blocking_queue.hpp"

namespace py = pybind11;

struct Socket_Data
{
  uint64_t stock_point;
};

int main()
{
  std::mutex mtx;

  py::scoped_interpreter guard{};
  py::gil_scoped_release release;

  Blocking_Queue<uint64_t> *stock_points =
    new Blocking_Queue<uint64_t>(1ull << 20);

  ix::WebSocket ws;

  std::string url("ws://localhost:1234/");
  ws.setUrl(url);

  ws.setPingInterval(45);

  fmt::print("connecting to {}...\n", url);

  ws.setOnMessageCallback([stock_points](const ix::WebSocketMessagePtr &msg)
  {
    if (msg->type == ix::WebSocketMessageType::Message)
    {
      uint64_t data = std::stoi(msg->str, nullptr, 10);
      stock_points->enqueue(data);
    }
  });

  ws.start();

  std::thread([&] {
    py::gil_scoped_acquire acquire;
    py::module_ algorithm = py::module_::import("algorithm");

    py::object speed = algorithm.attr("SPEED");
    int speed_ms = speed.cast<int>();

    while (true)
    {
      int count = 0;

      std::this_thread::sleep_for(std::chrono::milliseconds(speed_ms));
      std::lock_guard<std::mutex> _(mtx);
      if (!stock_points->empty())
      {
        int point = stock_points->dequeue();
        py::object result = algorithm.attr("calc_point")(point);
        fmt::print("data: {} -> {}\n", point, result.cast<int>());
        count++;

        if (count >= 100)
        {
          break;
        }
      }
    }
  }).detach();

  std::string text;
  while (std::getline(std::cin, text))
  {
    if (text == "exit")
    {
      ws.stop();
      exit(0);
    }
  }

  return 0;
}

