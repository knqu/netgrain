#define CROW_ENABLE_SSL

#include "crow.h"
#include "crow/middlewares/cookie_parser.h"
#include "crow/middlewares/session.h"
#include "../../../database/connector.cpp"
#include "../../../core/src/generator/generator.hpp"
#include <mailio/message.hpp>
#include <mailio/smtp.hpp>

#include <map>
#include <random>
#include <string>
#include <fstream>
#include <iostream>

int main() {
    using Session = crow::SessionMiddleware<crow::InMemoryStore>;
    crow::App<crow::CookieParser, Session> app{Session{
        crow::CookieParser::Cookie("session").max_age(129600).path("/"),
        4,
        crow::InMemoryStore{}
    }};

    crow::mustache::set_global_base("../../my-project/dist");
    
    std::unordered_set<crow::websocket::connection *> users;
    std::mutex mtx;

    Generator global_gen(0.02, 2, 100, 100); // HX
    Data_Transfer parameters;
    parameters.conn.store(nullptr);
    parameters.gen.store(true);
    parameters.new_event.store(0);
    parameters.send_data.store(false);

    // MERGE ATTEMPT
    CROW_WEBSOCKET_ROUTE(app, "/websocket")
      .onopen([&](crow::websocket::connection &conn) {
        fmt::print("new websocket connection from {}!\n", conn.get_remote_ip());
        std::lock_guard<std::mutex> _(mtx);
        users.insert(&conn);
        parameters.conn.store(&conn);
        parameters.send_data.store(true);
      })
      .onclose([&]( // need to reset
        crow::websocket::connection &conn,
        const std::string &reason,
        uint16_t) {

        fmt::print("websocket connection closed: {}\n", reason);
        std::lock_guard<std::mutex> _(mtx);
        parameters.send_data.store(false);
        parameters.conn.store(nullptr);
        global_gen.reset();
        users.erase(&conn);
      })
      .onmessage([&](
        crow::websocket::connection &conn,
        const std::string &data,
        bool is_binary) {

        fmt::print("data received {}\n", data);

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
          fmt::print("sideways!\n");
          parameters.new_event.store(3);
        }

        if (data == "bear") { 
          fmt::print("bear market triggered!\n");
          parameters.new_event.store(4); 
        }

        if (data == "bull") { 
          fmt::print("bull market triggered!\n");
          parameters.new_event.store(5); 
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

        if (data.starts_with("update")) {
          int index = data.find(":");
          global_gen.overwrite(stod(data.substr(index + 2)));
        }
      });

    CROW_ROUTE(app, "/assets/index-<string>")([](std::string file) {
        crow::response res;
        res.set_static_file_info_unsafe("../../my-project/dist/assets/index-" + file);
        res.set_header("Access-Control-Allow-Origin", "https://localhost");
        res.set_header("Access-Control-Allow-Credentials", "true");
        return res;
    });

    CROW_ROUTE(app, "/login")([]() {
        auto page = crow::mustache::load_unsafe("index.html");
        return page.render();
    });

    CROW_ROUTE(app, "/api/cookieCheck").methods(crow::HTTPMethod::GET, crow::HTTPMethod::PATCH)([&](const crow::request& req) {
        auto& session = app.get_context<Session>(req);
        if (session.get<bool>("loggedIn") == true) {
            return crow::response(200);
        }
        
        return crow::response(400);
    });

    CROW_ROUTE(app, "/api/loginAttempt").methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)([&](const crow::request& req) {
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

    CROW_ROUTE(app, "/api/registration").methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)([&](const crow::request& req) {
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
            conn.authenticate("burnermonkeyeye@gmail.com", "sddm nwly whin hkqr", mailio::smtps::auth_method_t::START_TLS);
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

    CROW_ROUTE(app, "/api/verifyEmail").methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)([&](const crow::request& req) {
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

    CROW_ROUTE(app, "/api/forgotPassword").methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)([&](const crow::request& req) {
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
            conn.authenticate("burnermonkeyeye@gmail.com", "sddm nwly whin hkqr", mailio::smtps::auth_method_t::START_TLS);
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

    CROW_ROUTE(app, "/api/attemptDaily").methods(crow::HTTPMethod::POST, crow::HTTPMethod::Patch)([&](const crow::request& req) {
        auto reqBody = crow::json::load(req.body);
        try {
            auto& cookie = app.get_context<crow::CookieParser>(req);            
            ConnectorSingleton::getInstance().addLeaderboardAttempt(cookie.get_cookie("email"), reqBody["profit"].i(), reqBody["time"].s());
        } catch (...) {
            return crow::response(400);
        }
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/fetchLeaderboard").methods(crow::HTTPMethod::GET, crow::HTTPMethod::Patch)([&](const crow::request& req) {
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

    CROW_ROUTE(app, "/api/saveSim").methods(crow::HTTPMethod::GET, crow::HTTPMethod::Patch)([&](const crow::request& req) {
        // save to database
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/fetchSim").methods(crow::HTTPMethod::POST, crow::HTTPMethod::Patch)([&](const crow::request& req) {
        auto& cookie = app.get_context<crow::CookieParser>(req);
        auto reqBody = crow::json::load(req.body);
        
        try {
            return crow::response(200);
            std::string filePath = ConnectorSingleton::getInstance().fetchSimulation(reqBody["submitted_simID"].i(), cookie.get_cookie("email")).at(0);
            crow::response res;
            res.write("{filePath : " + filePath + "}");
            res.code = 200;
            return res;
        } catch (...) {
            return crow::response(400);
        }
    });

    CROW_ROUTE(app, "/api/saveLayout").methods(crow::HTTPMethod::POST, crow::HTTPMethod::Patch)([&](const crow::request& req) {
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

    CROW_ROUTE(app, "/api/fetchLayout").methods(crow::HTTPMethod::GET, crow::HTTPMethod::Patch)([&](const crow::request& req) {
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

    CROW_ROUTE(app, "/api/simAveraged").methods(crow::HTTPMethod::GET, crow::HTTPMethod::Patch)([&](const crow::request& req) { 
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

    CROW_ROUTE(app, "/api/calculateFee").methods(crow::HTTPMethod::GET, crow::HTTPMethod::Patch)([&](const crow::request& req) { 
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

    CROW_ROUTE(app, "/api/fetchHistory").methods(crow::HTTPMethod::GET, crow::HTTPMethod::Patch)([&](const crow::request& req) {
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

    CROW_CATCHALL_ROUTE(app)([](){
        std::cout << "Catch All" << std::endl;
        crow::response res;
        res.set_static_file_info_unsafe("../../my-project/src/components/catchall.html");
        return res;
    });

    std::thread([&]{
      global_gen.generate_ws(&parameters);
    }).detach();

    app.bindaddr("127.0.0.1").port(18080);
    //parameters.gen.store(false);

    app.run();
}
