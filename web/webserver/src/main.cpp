#include "crow.h";
#include "crow/middlewares/cookie_parser.h";
#include "crow/middlewares/session.h";
#include "../../../database/connector.cpp";

#include <map>;
#include <chrono>;
#include <random>;
#include <string>;
#include <iostream>;

int main() {
    typedef std::chrono::time_point<std::chrono::system_clock> Timepoint;
    
    using Session = crow::SessionMiddleware<crow::InMemoryStore>;
    crow::App<crow::CookieParser, Session> app{Session{
        crow::CookieParser::Cookie("session").max_age(129600).path("/"),
        4,
        crow::InMemoryStore{}
    }};

    crow::mustache::set_global_base("../../my-project/dist");

    std::map<std::string, Timepoint> validSession;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> randID(100000000, 999999999);

    CROW_ROUTE(app, "/")([]() {
        std::cout << "----------------Hi\n";
        auto page = crow::mustache::load_text_unsafe("index.html");
        return page;
    });

    CROW_ROUTE(app, "/assets/index-<string>")([](std::string file) {
        crow::response res;
        res.set_static_file_info_unsafe("../../my-project/dist/assets/index-" + file);
        return res;
    });

    CROW_ROUTE(app, "/api/loginAttempt").methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)([&](const crow::request& req) {
      auto& session = app.get_context<Session>(req);
      auto& cookie = app.get_context<crow::CookieParser>(req);
      auto reqBody = crow::json::load(req.body);

      std::string email = reqBody["login_submitted_email"].s();
      std::string password = reqBody["login_submitted_password"].s();

      std::cout << "Email: " << email << std::endl << "Password: " << password << std::endl;
      std::cout << cookie.get_cookie("email") << "\n";

      if (session.get<bool>(cookie.get_cookie("email")) == true) {
          return crow::response(200);
      }

      int dbResponse = ConnectorSingleton::getInstance().login(
          email,
          password
      );

     if (dbResponse == true) {
        crow::response res;
        res.set_header("Access-Control-Allow-Origin", "http://localhost:18080");
        res.set_header("Access-Control-Allow-Credentials", "true");
        session.set(email, true);
        cookie.set_cookie("email", email).max_age(120).path("/");
        res.code = 200;
        std::cout << "Success" << std::endl;
        return res;
      }
      else {
          std::cout << "Invalid" << std::endl;
          return crow::response(400);
      }
    });


    /*CROW_ROUTE(app, "/api/signupAttempt").methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)([](const crow::request& req)) {
        auto reqBody = crow::json::load(req.body);
        std::string registeredEmail = reqBody['registration_submitted_email'].s();
        std::string registeredPassword = reqBody['registration_submitted_password'].s();

        
    } */

    app.port(18080).run();
}
