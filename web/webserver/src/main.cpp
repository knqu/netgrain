#include <fmt/core.h>
#include <crow.h>
#include "crow/middlewares/cookie_parser.h"
#include "crow/middlewares/session.h"
#include "../../../database/connector.cpp"
#include "../../../lib/nlohmann/json.hpp"
#include "../../../core/src/generator/generator.hpp"
#include "../../../core/src/simulator/simulator.hpp"
#include <mailio/message.hpp>
#include <mailio/smtp.hpp>

#include <random>
#include <string>
#include <fstream>
#include <iostream>
#include <memory>
#include <string_view>
#include <algorithm>
#include <cstdio>
#include <filesystem>

using njson = nlohmann::json; // HACK:
MarketDataManager data_manager;

void load_ticker_data() {
    string default_dir = "../../../data/";  // NOTE: updated to work when run from webserver directory

    if (std::filesystem::exists(default_dir) && std::filesystem::is_directory(default_dir)) {
        for (const auto& entry : filesystem::recursive_directory_iterator(default_dir)) {
        // recursive bc I changed to have subfolders for asset classes
            if (entry.is_regular_file()) {
                std::string file_path = entry.path().string();
                std::string file_name = entry.path().filename().string();
                //name of the folder is assect class
                std::string asset_class = entry.path().parent_path().filename().string();
                if (asset_class == "data") asset_class = "Stocks";

                std::string ticker = file_name.substr(0, file_name.find('.'));
                transform(
                    ticker.begin(), ticker.end(),
                    ticker.begin(), [](unsigned char c){ return toupper(c); }
                );
                data_manager.load_ticker_data(ticker, file_path, asset_class);
            }
        }
        cout << "Success Loading in stored data\n";
    } else {
        cout << "Data directory not found: " << default_dir << "\n";
    }
}

int main() {
    load_ticker_data();
    using Session = crow::SessionMiddleware<crow::InMemoryStore>;
    crow::App<crow::CookieParser, Session> app{Session{
        crow::CookieParser::Cookie("session").max_age(129600).path("/"),
        4,
        crow::InMemoryStore{}
    }};

    crow::mustache::set_global_base("../../my-project/dist");

    std::unordered_set<crow::websocket::connection *> users;
    std::mutex mtx;

    std::vector<std::unique_ptr<Generator>> generators;

    Generator global_gen(0.02, 2, 100, 100, 0);

    CROW_ROUTE(app, "/api/market").methods(
        crow::HTTPMethod::GET)([&](const crow::request& req) {

        string json = data_manager.get_market_state_json();
        crow::response res(json);
        res.code = 200;
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");
        return res;
    });

    CROW_ROUTE(app, "/api/upload").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::PATCH)([&](const crow::request& req) {

        string filename(req.get_header_value("x-file-name"));
        string asset_class(req.get_header_value("x-asset-class"));

        if (filename.empty()) {
            filename = "unknown_upload.csv";
        }

        if (asset_class.empty()) {
            asset_class = "Custom";
        }

        std::string save_path = "temp_data/" + filename;
        ofstream out_file(save_path, ios::binary);

        if (out_file.is_open()) {
            out_file << req.body;
            out_file.close();
            cout << "Saved file: " << save_path << " (" << req.body.size() << " bytes)\n";
            string ticker = filename.substr(0, filename.find('.'));
            transform(
                ticker.begin(), ticker.end(),
                ticker.begin(), [](unsigned char c){ return toupper(c); }
            );

            // DUPLICATE HANDLING
            if (data_manager.has_ticker(ticker)) {
                std::cout << "D" << std::endl;
                std::cout << "Duplicate " << ticker << " already in map.\n";
                remove(save_path.c_str());

                crow::response res("Error: " + ticker + " already exists.");
                res.set_header("Access-Control-Allow-Origin", "*");
                res.code = 409;
                return res;
            }

            //CHANGED TO BOOLEAN TO PROPOGATE CHANGES TO SIMULATION.TSX
            bool load_success = data_manager.load_ticker_data(ticker, save_path, asset_class);

            if (load_success) {
                data_manager.print_first_row(ticker);
            }

            if (remove(save_path.c_str()) == 0) {
                std::cout << "Removed File after loading (save it in temp, read, delete): "
                          << save_path << "\n";
            } else {
                std::cout << "Error in removing file: " << save_path << "\n";
            }

            //message based on load_success
            if (load_success) {
                crow::response res(ticker);
                res.set_header("Access-Control-Allow-Origin", "*");
                res.code = 200;
                return res;
            } else {
                crow::response res("Invalid or corrupted CSV data");
                res.code = 400;
                return res;
            }
        } else {
            crow::response res("Failed to save to disk.");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.code = 500;
            return res;
        }
        crow::response res("Error");
        return res;
    });

    CROW_ROUTE(app, "/api/simulate").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::PATCH,
        crow::HTTPMethod::OPTIONS
        )([&](const crow::request& req) {

        // CORS Preflight
        if (req.method == crow::HTTPMethod::OPTIONS) {
            crow::response res(200);
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "POST, PATCH, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type");
            return res;
        }

        // Parse input JSON data
        njson j;

        try {
            j = njson::parse(req.body); //HACK:
        } catch (const njson::parse_error& e) {
            std::cout << "ERROR\n";
            crow::response res(R"({"error": "Invalid JSON"})");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Content-Type", "application/json");
            res.code = 500;
            return res;
        }

        // Grab values from a few parameters (default to second parameter of .value() if not found)
        std::string mode = j.value("mode", "generated");
        i64 initial_capital = static_cast<i64>(
                j.value("initial_capital", 10000.0) * PRICE_SCALE_FACTOR);
        int num_bars = j.value("num_bars", 252);

        struct TickerEntry {
            std::string name;
            int base_price = 100;
            int volatility = 20;
            int liquidity = 1000;
            int market_cap = 5000;
        };

        std::vector<TickerEntry> ticker_entries;

        // Error check: if no tickers are found
        if (!j.contains("stocks") || !j["stocks"].is_array()) {
            crow::response res(R"({"error": "No tickers provided"})");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Content-Type", "application/json");
            res.code = 500;
            return res;
        }

        // For each ticker, add to ticker_entries
        // Example:
        // {
        //   "initial_capital":100000,
        //   "stocks":[
        //     {
        //       "ticker":"GOOGL",
        //       "base_price":135,
        //       "liquidity":50,
        //       "volatility":2,
        //       "market_cap":3
        //     }
        //   ],
        //   "start_date":"",
        //   "end_date":"",
        //   "trade_fee":1.5,
        //   "script":""
        // }
        for (auto& stock : j["stocks"]) {
            TickerEntry entry;
            entry.name       = stock.at("ticker").get<std::string>();
            entry.base_price = stock.value("base_price", 100);
            entry.liquidity  = stock.value("liquidity", 1000);
            entry.volatility = stock.value("volatility", 20);
            entry.market_cap = stock.value("market_cap", 5000);
            ticker_entries.push_back(entry);
        }

        // Error Check: if no tickers were parsed into array
        if (ticker_entries.empty()) {
            crow::response res(R"({"error": "No tickers provided"})");
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Content-Type", "application/json");
            res.code = 500;
            return res;
        }

        Engine engine(initial_capital);
        SimulationResult result;

        int id = 0;

        auto& cookie = app.get_context<crow::CookieParser>(req);
        std::string email = cookie.get_cookie("email");
        std::cerr << email << std::endl;

        // Generate Mode: convert ticker entries to generators
        // NOTE: run_generated_simulation() seems to basically call gbm()
        // for num_bars (finite) and outputs SimulationResult
        if (mode == "generated") {
            for (const auto& entry : ticker_entries) {
                generators.push_back(std::make_unique<Generator>(
                            entry.name, entry.base_price, entry.volatility,
                            entry.liquidity, entry.market_cap, id++));

                Generator* gen_ptr = generators.back().get();
                ConnectorSingleton::getInstance().createSimulation(email, "", -1);

                std::thread([gen_ptr]{
                    gen_ptr->generate_ws();
                }).detach();
            }
            result = run_generated_simulation(engine, generators, num_bars);
        } else {  // csv mode
            std::vector<std::string> tickers;
            for (const auto& entry : ticker_entries) {

                // Error Check: verify all tickers are loaded
                if (!data_manager.has_ticker(entry.name)) {
                    crow::response res(
                        "{\"error\": \"Ticker data not found: " + entry.name + "\"}");
                    res.set_header("Access-Control-Allow-Origin", "*");
                    res.set_header("Content-Type", "application/json");
                    res.code = 500;
                    return res;
                }
                tickers.push_back(entry.name);
            }
            result = run_csv_simulation(engine, data_manager, tickers);
        }

        std::string output = serialize_simulation_result(result);

        crow::response res(output);
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Content-Type", "application/json");
        res.code = 200;
        return res;
    });

    CROW_WEBSOCKET_ROUTE(app, "/websocket")
        .onopen([&](crow::websocket::connection &conn) {
            fmt::print("new websocket connection from {}!\n", conn.get_remote_ip());
            std::lock_guard<std::mutex> _(mtx);
            users.insert(&conn);

            global_gen.gen_settings.conn.store(&conn);
            global_gen.gen_settings.send_data.store(true);

            if (generators.size()) {
                global_gen.gen_settings.send_data.store(false);
                global_gen.gen_settings.conn.store(nullptr);
                global_gen.reset();

                for (const auto& x : generators) {
                    x->gen_settings.conn.store(&conn);
                    x->gen_settings.send_data.store(true);
                }
            }
        })
        .onclose([&]( // need to reset
            crow::websocket::connection &conn,
            const std::string &reason,
            uint16_t) {

            fmt::print("websocket connection closed: {}\n", reason);
            std::lock_guard<std::mutex> _(mtx);

            if (!generators.size()) {
                global_gen.gen_settings.send_data.store(false);
                global_gen.gen_settings.conn.store(nullptr);
                global_gen.reset();
            }
            users.erase(&conn);
        })
        .onmessage([&](
            crow::websocket::connection &conn,
            const std::string &data,
            bool is_binary) {

            fmt::print("data received {}\n", data);

            std::lock_guard<std::mutex> _(mtx);

            // NOTE: to avoid drastic changes, only websocket connections with
            // "event (X)", where X is the ID will be processed different, all
            // others will still use global_gen
            // NOTE: after /api/simulate, let generators vector be filled and be a separate case

            // <<<<<<<<< EVENTS >>>>>>>>>>>>
            if (data.starts_with("sim")) {
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

                generators.push_back(
                    std::make_unique<Generator>(drift, volatility, price, target, 1));

                fmt::print("[id: {}]: {} {} {} {}\n",
                    generators.size(),
                    drift,
                    volatility,
                    price,
                    target
                );

                std::thread([&]{
                    global_gen.generate_ws();
                }).detach();
            }
            else if (data == "flash_crash") {
                if (global_gen.gen_settings.new_event.load() == 0) {
                    global_gen.gen_settings.new_event.store(1);
                    fmt::print("flash crash!\n");
                }
            }
            else if (data == "sideways") {
                fmt::print("sideways!\n");
                global_gen.gen_settings.new_event.store(3);
            }
            else if (data == "bear") {
                fmt::print("bear market triggered!\n");
                global_gen.gen_settings.new_event.store(4);
            }
            else if (data == "bull") {
                fmt::print("bull market triggered!\n");
                global_gen.gen_settings.new_event.store(5);
            }
            else if (data == "stop") {
                global_gen.gen_settings.send_data.store(false);
            }
            else if (data.starts_with("bubble")) {
                if (global_gen.gen_settings.new_event.load() == 0) {
                    int threshold = std::stoi(data.substr(7), nullptr, 10);
                    global_gen.gen_settings.new_event.store(2);
                    global_gen.gen_settings.threshold.store(threshold);
                    fmt::print("bubble! {}\n", threshold);
                }
                else if (global_gen.gen_settings.new_event.load() == 2) {
                    fmt::print("bubble is ignored: called consecutively when another is active!\n");
                }
            }
            else if (data == "pause") {
                global_gen.gen_settings.pause.store(true);
            }
            else if (data == "resume") {
                global_gen.gen_settings.pause.store(false);
            }
            else if (data.starts_with("freq:")) {
                int interval = std::stoi(data.substr(5));
                global_gen.gen_settings.snapshot_interval.store(interval);
                for (auto& g : generators) {
                    g->gen_settings.snapshot_interval.store(interval);
                }
            }
            else if (data.starts_with("update")) {
                int index = data.find(":");
                global_gen.overwrite(stod(data.substr(index + 2)));
            }
            else if (data.starts_with("rewind")) {
                if (global_gen.gen_settings.new_event.load() == 0) {
                    int rewind_count = std::stoi(data.substr(7), nullptr, 10);
                    rewind_count = std::min<int>(rewind_count, global_gen.streamed_points->size());
                    double last_price_point =
                        global_gen.streamed_points->at(
                            global_gen.streamed_points->size() - rewind_count);

                    global_gen.streamed_points->erase(
                        global_gen.streamed_points->end() - rewind_count,
                        global_gen.streamed_points->end()
                    );
                    global_gen.gen_settings.reset_price.store(last_price_point);
                    global_gen.gen_settings.new_event.store(6);
                }
                else if (global_gen.gen_settings.new_event.load() == 2) {
                    fmt::print("rewind is ignored: another event is active\n");
                }
            }
            else if (data.starts_with("set_fields"))
            {
                std::string fields_str = data.substr(11);

                int find_pos = fields_str.find_first_of("~");
                int base_price = std::stoi(fields_str.substr(0, find_pos));

                fields_str = fields_str.substr(find_pos + 1);
                find_pos = fields_str.find_first_of("~");
                double percent_drift =
                    std::stod(fields_str.substr(0, find_pos));

                fields_str = fields_str.substr(find_pos + 1);
                find_pos = fields_str.find_first_of("~");
                double percent_volatility =
                    std::stod(fields_str.substr(0, find_pos));

                fields_str = fields_str.substr(find_pos + 1);
                find_pos = fields_str.find_first_of("~");
                int market_cap = std::stoi(fields_str.substr(0, find_pos));

                fields_str = fields_str.substr(find_pos + 1);
                int target_price = std::stoi(fields_str);

                global_gen.set_fields(base_price, percent_drift, percent_volatility,
                        market_cap, target_price);
            }
            else if (data.starts_with("multiple")) {
                int index = data.find("(");
                // TODO:
            }
        });

    CROW_ROUTE(app, "/assets/<string>")([](std::string filename) {
        crow::response res;

        std::string path = "../../my-project/dist/assets/" + filename;
        res.set_static_file_info_unsafe(path);

        if (filename.ends_with(".js")) res.set_header("Content-Type", "application/javascript");
        else if (filename.ends_with(".css")) res.set_header("Content-Type", "text/css");
        else if (filename.ends_with(".svg")) res.set_header("Content-Type", "image/svg+xml");

        res.set_header("Access-Control-Allow-Origin", "*");
        return res;
    });

    CROW_ROUTE(app, "/login")([]() {
        auto page = crow::mustache::load_unsafe("index.html");
        return page.render();
    });

    CROW_ROUTE(app, "/api/cookieCheck").methods(
        crow::HTTPMethod::GET,
        crow::HTTPMethod::PATCH)([&](const crow::request& req) {

        auto& session = app.get_context<Session>(req);
        if (session.get<bool>("loggedIn") == true) {
            return crow::response(200);
        }

        return crow::response(400);
    });

    CROW_ROUTE(app, "/api/loginAttempt").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::PATCH)([&](const crow::request& req) {

        auto& session = app.get_context<Session>(req);
        auto reqBody = crow::json::load(req.body);

        std::string email = reqBody["login_submitted_email"].s();
        std::string password = reqBody["login_submitted_password"].s();

        int dbResponse = ConnectorSingleton::getInstance().login(email, password) ;

        if (dbResponse == true) {
            auto& cookie = app.get_context<crow::CookieParser>(req);
            cookie.set_cookie("email", email).max_age(129600).path("/");
            session.set("loggedIn", true);
            crow::response res;
            res.code = 200;
            return res;
        }
        else {
            return crow::response(400);
        }
    });

    CROW_ROUTE(app, "/api/registration").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::PATCH)([&](const crow::request& req) {

        auto& session = app.get_context<Session>(req);
        auto reqBody = crow::json::load(req.body);

        std::string registeredEmail = reqBody["registration_submitted_email"].s();
        std::string registeredPassword = reqBody["registration_submitted_password"].s();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> randID(0, 9);
        std::string sixDigits = "";
        for (int i = 0; i < 6; i++) {
            sixDigits += std::to_string(randID(gen));
        }

        session.set("registeredEmail", registeredEmail);
        session.set("registeredPassword", registeredPassword);
        session.set("sixDigits", sixDigits);
        try {
            mailio::mail_address sender("mailio library", "burnermonkeyeye@gmail.com");
            mailio::mail_address recipient("mailio library", registeredEmail);

            mailio::message msg;
            msg.from(sender);
            msg.add_recipient(recipient);
            msg.subject("NetGrain Registration");
            msg.content(std::string("You Verifcation Code is: ") + sixDigits);

            mailio::smtps conn("smtp.gmail.com", 587);
            conn.authenticate(
                "burnermonkeyeye@gmail.com",
                "sddm nwly whin hkqr",
                mailio::smtps::auth_method_t::START_TLS);
            conn.submit(msg);
        }
        catch (mailio::smtp_error& exc) {
            std::cout << exc.what() << std::endl;
        }
        catch (mailio::dialog_error& exc) {
            std::cout << exc.what() << std::endl;
        }
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/verifyEmail").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::PATCH)([&](const crow::request& req) {

        auto& session = app.get_context<Session>(req);
        auto reqBody = crow::json::load(req.body);

        std::string userCodeInput = reqBody["submitted_verification_code"].s();
        std::string code = session.get("sixDigits", "");

        if (userCodeInput == session.get("sixDigits", "")) {
            std::string email = session.get("registeredEmail", "");
            std::string password = session.get("registeredPassword", "");
            if (session.get<bool>("forgot") == true) {
                session.remove("forgot");
                ConnectorSingleton::getInstance().changePassword(email, password);
            }
            else {
                ConnectorSingleton::getInstance().addUser(email, password, email);
            }
            auto& cookie = app.get_context<crow::CookieParser>(req);
            cookie.set_cookie("email", email).max_age(129600).path("/");

            session.remove("registeredEmail");
            session.remove("registeredPassword");
            session.remove("sixDigits");
            session.set("loggedIn", true);
            return crow::response(200);
        }
        else {
            session.remove("registeredEmail");
            session.remove("registeredPassword");
            session.remove("sixDigits");
            return crow::response(400);
        }
    });

    CROW_ROUTE(app, "/api/forgotPassword").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::PATCH)([&](const crow::request& req) {

        auto& session = app.get_context<Session>(req);
        auto reqBody = crow::json::load(req.body);

        std::string registeredEmail = reqBody["submitted_email"].s();
        std::string newPassword = reqBody["submitted_new_password"].s();

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> randID(0, 9);
        std::string sixDigits = "";
        for (int i = 0; i < 6; i++) {
            sixDigits += std::to_string(randID(gen));
        }

        session.set("submitted_email", registeredEmail);
        session.set("submitted_new_password", newPassword);
        session.set("sixDigits", sixDigits);
        session.set("forgot", true);
        try {
            mailio::mail_address sender("mailio library", "burnermonkeyeye@gmail.com");
            mailio::mail_address recipient("mailio library", registeredEmail);

            mailio::message msg;
            msg.from(sender);
            msg.add_recipient(recipient);
            msg.subject("NetGrain Registration");
            msg.content(std::string("You Verifcation Code is: ") + sixDigits);

            mailio::smtps conn("smtp.gmail.com", 587);
            conn.authenticate(
                "burnermonkeyeye@gmail.com",
                "sddm nwly whin hkqr",
                mailio::smtps::auth_method_t::START_TLS);
            conn.submit(msg);
        }
        catch (mailio::smtp_error& exc) {
            std::cout << exc.what() << std::endl;
        }
        catch (mailio::dialog_error& exc) {
            std::cout << exc.what() << std::endl;
        }
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/attemptDaily").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        auto reqBody = crow::json::load(req.body);
        try {
            auto& cookie = app.get_context<crow::CookieParser>(req);
            ConnectorSingleton::getInstance().addLeaderboardAttempt(
                cookie.get_cookie("email"), reqBody["profit"].i(), reqBody["time"].s());
        } catch (...) {
            return crow::response(400);
        }
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/fetchLeaderboard").methods(
        crow::HTTPMethod::GET,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        try {
            crow::response res;
            std::cout << "Fetching Leaderboard" << std::endl;
            std::string leaderboardJSON = ConnectorSingleton::getInstance().fetchLeaderBoard();
            std::cout << "Start: " << leaderboardJSON << std::endl;

            if (leaderboardJSON.size() == 0) {
                res.code = 201;
                return res;
            }

            res.set_header("Access-Control-Allow-Origin", "https://localhost");
            res.set_header("Access-Control-Allow-Credentials", "true");
            res.write(leaderboardJSON);
            res.code = 200;
            return res;
        } catch (...) {
            return crow::response(400);
        }
    });

    CROW_ROUTE(app, "/api/saveSim").methods(
        crow::HTTPMethod::GET,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        // save to database
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/fetchSim").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        auto& cookie = app.get_context<crow::CookieParser>(req);
        auto reqBody = crow::json::load(req.body);

        try {
            return crow::response(200);
            std::string filePath = ConnectorSingleton::getInstance().fetchSimulation(
                reqBody["submitted_simID"].i(), cookie.get_cookie("email")).at(0);
            crow::response res;
            res.write("{filePath : " + filePath + "}");
            res.code = 200;
            return res;
        } catch (...) {
            return crow::response(400);
        }
    });

    CROW_ROUTE(app, "/api/loadSim").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        std::string body = req.body;
        global_gen.gen_settings.pause.store(true);
        global_gen.load_simulation(body);
        global_gen.gen_settings.pause.store(true);

        global_gen.refresh();

        fmt::print("Loaded simulation!\n");

        std::vector<double> data_points = *(global_gen.streamed_points);

        crow::json::wvalue json_array;
        json_array["data"] = data_points;

        json_array["fields"]["base_price"] = global_gen.base_price;
        json_array["fields"]["percent_drift"] = global_gen.percent_drift;
        json_array["fields"]["percent_volatility"] = global_gen.percent_volatility;
        json_array["fields"]["market_cap"] = global_gen.market_cap;
        json_array["fields"]["target_price"] = global_gen.target_price;

        return crow::response(json_array);
    });

    CROW_ROUTE(app, "/api/serializeSim").methods(
        crow::HTTPMethod::GET,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        char *streamed_length = req.url_params.get("length");
        int streamed_len = stoi(streamed_length);

        fmt::print("streamed_len: {}\n", streamed_len);

        std::vector<char> file_buf;
        file_buf = global_gen.save_simulation(streamed_len);
        std::string file_binary(file_buf.begin(), file_buf.end());

        crow::response res;
        res.set_header("Content-Type", "application/octet-stream");
        res.body = file_binary;

        return res;
    });

    CROW_ROUTE(app, "/api/saveLayout").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        auto& cookie = app.get_context<crow::CookieParser>(req);
        std::string email = cookie.get_cookie("email");

        if (email.empty()) return crow::response(401);

        try {
            ConnectorSingleton::getInstance().linkCustomGUILayout(email, req.body);
            return crow::response(200);
        } catch (...) {
            return crow::response(500);
        }
    });

    CROW_ROUTE(app, "/api/fetchLayout").methods(
        crow::HTTPMethod::GET,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        auto& cookie = app.get_context<crow::CookieParser>(req);
        std::string email = cookie.get_cookie("email");

        if (email.empty()) return crow::response(401);

        try {
            std::string layoutJSON = ConnectorSingleton::getInstance().getCustomGUILayout(email);
            crow::response res;
            res.write(layoutJSON.empty() ? "{}" : layoutJSON);
            res.set_header("Content-Type", "application/json");
            res.code = 200;
            return res;
        } catch (...) {
            return crow::response(500);
        }
    });

    CROW_ROUTE(app, "/api/simAveraged").methods(
        crow::HTTPMethod::GET,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        auto& cookie = app.get_context<crow::CookieParser>(req);
        std::string email = cookie.get_cookie("email");
        if (email.empty()) return crow::response(401);

        try {
            std::string result = ConnectorSingleton::getInstance().average(email);
            std::cout << result << std::endl;
            crow::response res;
            res.write(result.empty() ? "" : result);
            res.set_header("Content-Type", "text/csv");
            res.code = 200;
            return res;
        } catch (...) {
            return crow::response(500);
        }
    });

    CROW_ROUTE(app, "/api/calculateFee").methods(
        crow::HTTPMethod::GET,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {
        auto& cookie = app.get_context<crow::CookieParser>(req);
        std::string email = cookie.get_cookie("email");

        if (email.empty()) return crow::response(401);

        try {
            std::string result = ConnectorSingleton::getInstance().fees(email, 1);
            crow::response res;
            res.write(result.empty() ? "" : result);
            res.set_header("Content-Type", "text/plain");
            res.code = 200;
            return res;
        } catch (...) {
            return crow::response(500);
        }
    });

    CROW_ROUTE(app, "/api/fetchHistory").methods(
        crow::HTTPMethod::GET,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        auto& cookie = app.get_context<crow::CookieParser>(req);
        std::string email = cookie.get_cookie("email");

        if (email.empty()) return crow::response(401);

        try {
            std::vector<int> hist = ConnectorSingleton::getInstance().fetchAllSims(email);

            if (hist.size() == 0) {
                crow::response res;
                res.code = 201;
                return res;
            }

            std::string jsonStr = "[";
            for (int i = 0; i < hist.size(); i++) {
                std::string filePath = "../src/sims/" + std::to_string(hist[i]) + "/simResults";
                std::vector<std::string> fileInfo(4);
                std::ifstream file(filePath);
                for (int j = 0; j < 4; j++) {
                    std::getline(file, fileInfo[j]);
                }
                file.close();

                jsonStr += "{";
                jsonStr += "\"SimName\":\"" + fileInfo[0] + "\",";
                jsonStr += "\"Date\":\"" + fileInfo[1] + "\",";
                jsonStr += "\"Duration\":\"" + fileInfo[2] + "\",";
                jsonStr += "\"Profit\":\"" + fileInfo[3] + "\",";
                jsonStr += "\"ID\":" + std::to_string(hist[i]);
                jsonStr += "},";
            }
            jsonStr.pop_back();
            jsonStr += "]";

            crow::response res;
            res.code = 200;
            res.set_header("Access-Control-Allow-Origin", "https://localhost");
            res.set_header("Access-Control-Allow-Credentials", "true");
            res.set_header("Content-Type", "application/json");
            res.write(jsonStr);
            return res;
        } catch (...) {
            return crow::response(500);
        }
    });

    CROW_ROUTE(app, "/api/resultsTemplate").methods(
        crow::HTTPMethod::POST,
        crow::HTTPMethod::Patch)([&](const crow::request& req) {

        std::cout << "is this working" << std::endl;
        auto reqBody = crow::json::load(req.body);
        int simID = reqBody["simID"].i();
        std::cout << "simID" << std::endl;
        std::cout<< simID << std::endl;

        std::unordered_map<std::string, std::string> metrics;
        try {
            metrics = ConnectorSingleton::getInstance().fetchMetrics(simID);
        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            return crow::response(500);
        }

        auto& cookie = app.get_context<crow::CookieParser>(req);
        std::string email = cookie.get_cookie("email");
        std::vector<int> hist = ConnectorSingleton::getInstance().fetchAllSims(email);
        njson j;

        if (hist.size() >= 2) {
            std::vector<double> q = ConnectorSingleton::getInstance().comparativeAnalytics(
                    hist.at(hist.size() - 1),
                    hist.at(hist.size() - 2)
                    );
            j["percent"] = std::to_string(q.at(q.size() - 3));
            j["flat"] = std::to_string(q.at(q.size() - 2));
            j["taxes"] = std::to_string(q.at(q.size() - 1));
        }
        try {
            /*
            j["table"] = njson::parse(metrics["table"]);
            j["equity"] = njson::parse(metrics["equity"]);
            j["pl"] = njson::parse(metrics["PL"]);
            j["drawdown"] = njson::parse(metrics["drawdown"]);
            */
            j["table"] = njson::array();
            j["equity"] = njson::array();
            j["pl"] = njson::array();
            j["drawdown"] = njson::array();
        } catch (const njson::parse_error& e) {
            std::cerr << "JSON Parse error: " << e.what() << std::endl;
        }

        crow::response res(j.dump());
        res.set_header("Content-Type", "application/json");
        res.code = 200;
        return res;
    });

    CROW_CATCHALL_ROUTE(app)([](){
        std::cout << "Catch All" << std::endl;
        crow::response res;
        res.set_static_file_info_unsafe("../../my-project/src/components/catchall.html");
        return res;
    });

    std::thread([&]{
        global_gen.generate_ws();
    }).detach();

    app.bindaddr("127.0.0.1").port(18080).multithreaded().run();


    /* Original Code prior to multithreading attempt
    std::thread([&]{
        global_gen.generate_ws(&parameters, streamed_points);
    }).detach();

    app.bindaddr("127.0.0.1").port(18080);
    //parameters.gen.store(false);
    app.run();
    */
}
