#include <drogon/HttpSimpleController.h>
using namespace drogon;

class HelloWorld : public HttpSimpleController<HelloWorld> {
public:
    void asyncHandleHttpRequest(const HttpRequestPtr& req, std::function<void (const HttpResponsePtr&)> &&callback) override {
        auto resp = HttpResponse::newHttpResponse();
        resp->setBody("<h1>Hello World from Drogon!</h1>");
        callback(resp);
    }
    PATH_LIST_BEGIN
    PATH_ADD("/", Get); 
    PATH_LIST_END
};
