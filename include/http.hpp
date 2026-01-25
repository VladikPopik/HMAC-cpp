#pragma once

#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <atomic>
#include "logging.hpp"
#include "config.hpp"
#include "hmac.hpp"

namespace service::micro {

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

using namespace service::config;
using namespace service::logging;
using namespace service::hmac;

class Service {

public:
    Service(std::fstream& fs) : config_(Config(fs)) {
        listener_ = http_listener(config_.GetListen() + "/ping");
        listener_sign_ = http_listener(config_.GetListen() + "/sign");
        listener_verify_ = http_listener(config_.GetListen() + "/verify");

        
        setup_endpoints();

    }

    ~Service() = default;
    
    Service(const Service&) = delete;
    Service operator=(const Service&) = delete;
    
    Service(Service&&) = delete;
    Service& operator=(Service&&) = delete;
    
    void start();
    
    void stop();
    
private:
    http_listener listener_;
    http_listener listener_sign_;
    http_listener listener_verify_;

    std::atomic<int> request_count_{0};
    Config config_;
    

    void setup_endpoints() {

        Logger logger(config_.GetLogLevel());

        listener_.support(methods::GET, [this](http_request request) {
            return handle_ping(std::move(request));
        });

        logger.info("Method GET ping/");

        listener_sign_.support(methods::POST, [this](http_request request) {
            handle_sign(std::move(request));
        });

        logger.info("Method POST sign/");
        
        listener_verify_.support(methods::POST, [this](http_request request) {
            handle_verify(std::move(request));
        });

        logger.info("Method POST verify/");
    }
    
    void handle_ping(http_request request) {
        Logger logger(config_.GetLogLevel());

        try {
        
        http_response response(200);

        json::value response_body;

        response_body[U("status")] = json::value::string(U("ALIVE"));
        response.set_body(response_body);
        request.reply(response);
        logger.info("Ping handled successfully");
        } catch (const std::exception& e) {
            logger.error("Exception in ping handler: " + std::string(e.what()));
                
            json::value error;
            error[U("error")] = json::value::string(U("Internal server error"));
            error[U("message")] = json::value::string(utility::conversions::to_string_t(e.what()));
            
            request.reply(status_codes::InternalError, error);
        }
    }

    void handle_sign(http_request request) {
        Logger logger(config_.GetLogLevel());

        try {
            request.extract_json()
            .then([=](json::value body) {
                auto msg = body.at(U("msg")).as_string();

                auto hmac = Hmac(config_.GetSecret());

                auto sig = hmac.Sign(std::move(msg));

                json::value response;
                response[U("signature")] = json::value::string(sig);
                
                request.reply(status_codes::OK, response);

            })
            .wait();
        }catch (const std::exception& e) {
            logger.error("Exception in sign handler: " + std::string(e.what()));
                
            json::value error;
            error[U("error")] = json::value::string(U("Internal server error"));
            error[U("message")] = json::value::string(utility::conversions::to_string_t(e.what()));
            
            request.reply(status_codes::InternalError, error);
        }
        logger.info("Sign handled successfully");

    }

    void handle_verify(http_request request) {
        Logger logger(config_.GetLogLevel());

        try {
            request.extract_json()
            .then([=](json::value body) {
                auto msg = body.at(U("msg")).as_string();
                auto sig = body.at(U("signature")).as_string();

                auto hmac = Hmac(config_.GetSecret());

                auto is_ok = hmac.Verify(std::move(msg), std::move(sig));

                json::value response;
                response[U("ok")] = json::value::boolean(is_ok);
                
                request.reply(status_codes::OK, response);
            })
            .wait();
        }catch (const std::exception& e) {
            logger.error("Exception in sign handler: " + std::string(e.what()));
                
            json::value error;
            error[U("error")] = json::value::string(U("Internal server error"));
            error[U("message")] = json::value::string(utility::conversions::to_string_t(e.what()));
            
            request.reply(status_codes::InternalError, error);
        }
        logger.info("Verify handled successfully");
    }
    
};

}