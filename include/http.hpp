#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <atomic>


namespace service::micro {

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class Service {

public:
    Service(const std::string& address) : listener_(address) {
        setup_endpoints();
    }
    
    void start();
    
    void stop();
    
private:
    http_listener listener_;
    std::atomic<int> request_count_{0};
    

    void setup_endpoints() {
        listener_.support(methods::GET, [this](http_request request) {
            handle_sign(std::move(request));
        });
        
        listener_.support(methods::POST, [this](http_request request) {
            handle_sign(std::move(request));
        });
    }
    
    void handle_sign(http_request request) {
        json::value response;

        request.body();
    }

    void handle_health_check(http_request request) {
        request_count_++;
        
        json::value response;
        response[U("status")] = json::value::string(U("OK"));
        
        request.reply(status_codes::OK, response);
    }
    
};

}