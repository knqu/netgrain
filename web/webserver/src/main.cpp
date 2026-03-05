#include "crow.h";
#include "crow/middlewares/cookie_parser.h";
#include "crow/middlewares/session.h";
#include "../../../database/connector.cpp";

#include <map>;
#include <chrono>;
#include <random>;
#include <string>;

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
        auto page = crow::mustache::load_text_unsafe("index.html");
        return page;
    });

    CROW_ROUTE(app, "/assets/index-<string>")([](std::string file) {
        crow::response res;
        res.set_static_file_info_unsafe("../../my-project/dist/assets/index-" + file);
        return res;
    });

    CROW_ROUTE(app, "/api/loginAttempt").methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)([&](const crow::request& req) {
        //auto& session = app.get_context<Session>(req);
        auto reqBody = crow::json::load(req.body);
        crow::response res;

        std::string email = reqBody["login_submitted_email"].s();
        std::string password = reqBody["login_submitted_password"].s();

        std::cout << "Email: " << email << std::endl << "Password: " << password << std::endl;

        /*if (session.get<bool>(email) == true) {
            res.set_header("Location", "/home");
            return res;
        } */
        //session.set(email, true);
        

        std::cout << ConnectorSingleton::getInstance().login("user@example.com", "Password1!") << std::endl;
        int dbResponse = ConnectorSingleton::getInstance().login(
            email,
            password
        );

       if (dbResponse == true) {
            res.set_header("Location", "/home");
            res.code = 302;
            //res.write(ConnectorSingleton::getInstance().fetchLeadboard());
            std::cout << "Success" << std::endl;
            res.write("Success");
            return res;
        }
        else {
            std::cout << "Invalid" << std::endl;
            res.write("Fail");
            res.code = 401;
            return res;
        }
    });

    /*CROW_ROUTE(app, "/api/signupAttempt").methods(crow::HTTPMethod::POST, crow::HTTPMethod::PATCH)([](const crow::request& req)) {
        auto reqBody = crow::json::load(req.body);
        std::string registeredEmail = reqBody['registration_submitted_email'].s();
        std::string registeredPassword = reqBody['registration_submitted_password'].s();

        
    } */

    app.port(18080).run();
}