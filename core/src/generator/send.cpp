#include <crow.h>

#include "generator.hpp"

#include <unordered_set>
#include <chrono>
#include <algorithm>

void save_simulation(Generator *gen, Data_Transfer *params,
                     std::vector<double> *streamed_points)
{
  std::chrono::time_point now = std::chrono::system_clock::now();
  std::chrono::duration duration = now.time_since_epoch();
  size_t seconds_since_epoch = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
  std::hash<size_t> hashing;

  FILE *fp = fopen(
    fmt::format("{}.sim", hashing(seconds_since_epoch)).c_str(), "wb");

  double percent_drift = gen->get_percent_drift();
  double percent_volatility = gen->get_percent_volatility();
  double dt = gen->get_dt();
  std::string ticker = gen->get_ticker();
  size_t ticker_len = ticker.size();
  int base_price = gen->base_price;
  int volatility = gen->volatility;
  int liquidity = gen->liquidity;
  int market_cap = gen->market_cap;
  int target_price = gen->target_price;
  fwrite(&percent_drift, sizeof(double), 1, fp);
  fwrite(&percent_volatility, sizeof(double), 1, fp);
  fwrite(&dt, sizeof(double), 1, fp);
  fwrite(&ticker_len, sizeof(size_t), 1, fp);
  fwrite(ticker.c_str(), sizeof(char), strlen(ticker.c_str()), fp);
  fwrite(&base_price, sizeof(int), 1, fp);
  fwrite(&volatility, sizeof(int), 1, fp);
  fwrite(&liquidity, sizeof(int), 1, fp);
  fwrite(&market_cap, sizeof(int), 1, fp);
  fwrite(&target_price, sizeof(int), 1, fp);

  fwrite(&params->rate_per_second, sizeof(int), 1, fp);
  fwrite(&params->new_event, sizeof(int), 1, fp);
  fwrite(&params->n_vol, sizeof(double), 1, fp);
  fwrite(&params->n_drift, sizeof(double), 1, fp);
  fwrite(&params->n_price, sizeof(int), 1, fp);
  fwrite(&params->threshold, sizeof(int), 1, fp);
  fwrite(&params->pause, sizeof(bool), 1, fp);

  size_t buffer_len = streamed_points->size();
  fwrite(&buffer_len, sizeof(size_t), 1, fp);
  for (int i = 0; i < streamed_points->size(); i++)
  {
    fwrite(&streamed_points->at(i), sizeof(double), 1, fp);
  }

  fclose(fp);
}

void load_simulation(std::string file, Generator *gen,
                     Data_Transfer *params,
                     std::vector<double> *streamed_points)
{
  FILE *fp = fopen(fmt::format("{}.sim", file).c_str(), "rb");

  std::string ticker;
  size_t ticker_len;
  fread(&gen->percent_drift, sizeof(double), 1, fp);
  fread(&gen->percent_volatility, sizeof(double), 1, fp);
  fread(&gen->dt, sizeof(double), 1, fp);
  fread(&ticker_len, sizeof(size_t), 1, fp);
  char *ticker_buf = (char *) calloc(ticker_len, sizeof(char));
  fwrite(ticker_buf, sizeof(char), strlen(ticker.c_str()), fp);
  gen->ticker = std::string(ticker_buf);
  fread(&gen->base_price, sizeof(int), 1, fp);
  fread(&gen->volatility, sizeof(int), 1, fp);
  fread(&gen->liquidity, sizeof(int), 1, fp);
  fread(&gen->market_cap, sizeof(int), 1, fp);
  fread(&gen->target_price, sizeof(int), 1, fp);

  fread(&params->rate_per_second, sizeof(int), 1, fp);
  fread(&params->new_event, sizeof(int), 1, fp);
  fread(&params->n_vol, sizeof(double), 1, fp);
  fread(&params->n_drift, sizeof(double), 1, fp);
  fread(&params->n_price, sizeof(int), 1, fp);
  fread(&params->threshold, sizeof(int), 1, fp);
  fread(&params->pause, sizeof(bool), 1, fp);

  size_t buffer_len;
  fread(&buffer_len, sizeof(size_t), 1, fp);
  for (int i = 0; i < buffer_len; i++)
  {
    double point;
    fread(&point, sizeof(point), 1, fp);
    streamed_points->push_back(point);
  }

  fclose(fp);
}

int main(int argc, const char *argv[])
{
  crow::SimpleApp app;
  std::unordered_set<crow::websocket::connection *> users;
  std::mutex mtx;

  std::vector<double> *streamed_points = new std::vector<double>();

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

      if (data == "pause") {
        parameters.pause.store(true);
      }

      if (data == "resume") {
        parameters.pause.store(false);
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

      if (data.starts_with("rewind"))
      {
        if (parameters.new_event.load() == 0)
        {
          int rewind_count = std::stoi(data.substr(7), nullptr, 10);
          rewind_count = std::min<int>(rewind_count, streamed_points->size());
          double last_price_point =
            streamed_points->at(streamed_points->size() - rewind_count);
          streamed_points->erase(
            streamed_points->end() - rewind_count,
            streamed_points->end()
          );
          parameters.reset_price.store(last_price_point);
          parameters.new_event.store(6);
        }
        else if (parameters.new_event.load() == 2)
        {
          fmt::print("rewind is ignored: another event is active\n");
        }
      }
    });

  std::thread([&]{
    global_gen.generate_ws(&parameters, streamed_points);
  }).detach();

  if (argc > 1)
  {
    app.port(atoi(argv[1])).multithreaded().run();
  }
  else {
    app.port(5555).multithreaded().run();
  }

  parameters.gen.store(false);
}
