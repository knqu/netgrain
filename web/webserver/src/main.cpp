#include "crow.h"
#include "crow/middlewares/cookie_parser.h"
#include "crow/middlewares/session.h"
#include "../../../database/connector.cpp"
#include <mailio/message.hpp>
#include <mailio/smtp.hpp>

#include <map>
#include <random>
#include <string>

int main() {  
    using Session = crow::SessionMiddleware<crow::InMemoryStore>;
    crow::App<crow::CookieParser, Session> app{Session{
        crow::CookieParser::Cookie("session").max_age(129600).path("/"),
        4,
        crow::InMemoryStore{}
    }};

    crow::mustache::set_global_base("../../my-project/dist");

    CROW_ROUTE(app, "/")([]() {
        auto page = crow::mustache::load_text_unsafe("index.html");
        return page;
    });

    CROW_ROUTE(app, "/assets/index-<string>")([](std::string file) {
        crow::response res;
        res.set_static_file_info_unsafe("../../my-project/dist/assets/index-" + file);
        return res;
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
            session.set("loggedIn", true);
            crow::response res;
            res.code = 200;
            return res;
        }
        else {
            std::cout << "Invalid" << std::endl;
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

    CROW_ROUTE(app, "/api/verifyEmail").methods(crow::HTTPMethod::POST, crow::HTTPMethod::Patch)([&](const crow::request& req) {
        auto& session = app.get_context<Session>(req);
        auto reqBody = crow::json::load(req.body);

        std::string userCodeInput = reqBody["submitted_verification_code"].s();

        if (userCodeInput == session.get("sixDigits", "")) {
            std::string email = session.get("registeredEmail", "");
            std::string password = session.get("registeredPassword", "");

            if (session.get<bool>("forgot") == true) {
                session.remove("forgot");
                // databasemethod to change details
            }
            else {
                ConnectorSingleton::getInstance().addUser(email, password, email);
            }
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

    CROW_ROUTE(app, "/api/attemptDaily").methods(crow::HTTPMethod::GET, crow::HTTPMethod::Patch)([&](const crow::request& req) {
        auto reqBody = crow::json::load(req.body);
        try {
            ConnectorSingleton::getInstance().addLeaderboardAttempt(0, reqBody["profit"].i(), reqBody["time"].s());
        } catch (...) {
            return crow::response(400);
        }
        return crow::response(200);
    });

    CROW_ROUTE(app, "/api/fetchLeaderboard").methods(crow::HTTPMethod::GET, crow::HTTPMethod::Patch)([&](const crow::request& req) {
        std::string leaderboardJSON = ConnectorSingleton::getInstance().fetchLeaderBoard();
        crow::response res;
        res.write(leaderboardJSON);
        return res;
    });

    CROW_CATCHALL_ROUTE(app)([](){
        crow::response res;
        res.set_static_file_info_unsafe("../../my-project/dist/index.html");
        return res;
    });

    app.port(18080).run();
}
