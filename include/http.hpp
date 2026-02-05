#pragma once

#include "config.hpp"
#include "hmac.hpp"
#include "logging.hpp"
#include <atomic>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <regex>

namespace service::micro
{

    using namespace web;
    using namespace web::http;
    using namespace web::http::experimental::listener;

    using namespace service::config;
    using namespace service::logging;
    using namespace service::hmac;

    class Service
    {

    public:
        Service(Config &&config, std::string config_path) : config_(std::move(config)), config_path_(config_path)
        {
            listener_ping_ = http_listener(config_.GetListen() + "/ping");
            listener_settings_ = http_listener(config_.GetListen() + "/settings");
            listener_sign_ = http_listener(config_.GetListen() + "/sign");
            listener_verify_ = http_listener(config_.GetListen() + "/verify");

            setup_endpoints();
        }

        ~Service() = default;

        Service(const Service &) = delete;
        Service operator=(const Service &) = delete;

        Service(Service &&) = delete;
        Service &operator=(Service &&) = delete;

        void start();

        void stop();

    private:
        http_listener listener_ping_;
        http_listener listener_settings_;
        http_listener listener_sign_;
        http_listener listener_verify_;

        std::atomic<int> request_count_{0};

        std::filesystem::path config_path_;
        Config config_;

        const std::regex regex_ = std::regex("(^[A-Za-z0-9\\-_]+$)");

        struct IsValid
        {
            status_code code;
            json::value error;
        };

        bool isValidBase64Url(const std::string &s)
        {
            if (s.empty())
                return true;

            auto match = std::regex_match(s, regex_);

            if (!match)
            {
                return false;
            }

            if (s.length() % 4 == 1)
                return false;
            return true;
        }

        IsValid ValidateMsg(const std::string &msg)
        {

            if (msg.length() > config_.GetMaxSizeBytes())
            {
                json::value error;
                error[U("error")] = json::value::string(U("invalid_msg"));
                return {status_codes::RequestEntityTooLarge, error};
            }

            return {status_codes::OK, {}};
        }

        void setup_endpoints()
        {

            listener_ping_.support(methods::GET, [this](http_request request)
                                   { return handle_ping(std::move(request)); });

            {
                LOG_INFO(Logger(config_.GetLogFile()), "Method GET ping/");
            }

            listener_settings_.support(methods::GET, [this](http_request request)
                                       { return handle_settings_update(std::move(request)); });

            {
                LOG_INFO(Logger(config_.GetLogFile()), "Method GET settings/");
            }

            listener_sign_.support(methods::POST, [this](http_request request)
                                   { handle_sign(std::move(request)); });

            {
                LOG_INFO(Logger(config_.GetLogFile()), "Method POST sign/");
            }

            listener_verify_.support(methods::POST, [this](http_request request)
                                     { handle_verify(std::move(request)); });

            {
                LOG_INFO(Logger(config_.GetLogFile()), "Method POST verify/");
            }
        }

        void handle_ping(http_request request)
        {
            try
            {

                http_response response(200);

                json::value response_body;

                response_body[U("status")] = json::value::boolean(true);
                response.set_body(response_body);
                request.reply(response);
                {
                    LOG_INFO(Logger(config_.GetLogFile()), "Ping handled successfully");
                }
            }
            catch (const std::exception &e)
            {
                LOG_DEBUG(Logger(config_.GetLogFile()), "Exception in settings handler: " + std::string(e.what()));

                json::value error;
                error[U("error")] = json::value::string(U("internal"));

                request.reply(status_codes::InternalError, error);
            }
        }

        void handle_settings_update(http_request request)
        {
            try
            {
                http_response response(200);
                json::value response_body;

                config_ = Config(config_path_);

                response_body[U("status")] = json::value::boolean(true);
                response.set_body(response_body);
                request.reply(response);
                {
                    LOG_INFO(Logger(config_.GetLogFile()), "Settings update handled successfully");
                }
            }
            catch (const std::exception &e)
            {
                {
                    LOG_DEBUG(Logger(config_.GetLogFile()), "Exception in ping handler: " + std::string(e.what()));
                }

                json::value error;
                error[U("error")] = json::value::string(U("internal"));

                request.reply(status_codes::InternalError, error);
            }
        }

        void handle_sign(http_request request)
        {
            if (!request.headers().has(U("Content-Type")) ||
                request.headers()[U("Content-Type")].find(U("application/json")) == utility::string_t::npos)
            {
                json::value error;
                error[U("error")] = json::value::string(U("invalid_json"));
                request.reply(
                    status_codes::UnsupportedMediaType,
                    error);
                return;
            }

            try
            {
                request.extract_json()
                    .then([&](json::value body)
                          {
                              if (body.is_null())
                              {
                                  json::value error;
                                  error[U("error")] = json::value::string(U("invalid_msg"));
                                  request.reply(
                                      status_codes::BadRequest,
                                      error);
                                  return;
                              }

                              if (!body.has_field(U("msg")))
                              {
                                  json::value error;
                                  error[U("error")] = json::value::string(U("invalid_msg"));
                                  request.reply(status_codes::BadRequest, error);
                                  return;
                              }

                              auto msg = body.at(U("msg")).as_string();

                              if (msg.length() == 0)
                              {
                                  json::value error;
                                  error[U("error")] = json::value::string(U("invalid_msg"));
                                  request.reply(status_codes::BadRequest, error);
                                  return;
                              }

                              IsValid is_valid;

                              if (!isValidBase64Url(msg))
                              {
                                  json::value error;
                                  error[U("error")] = json::value::string(U("invalid_signature_format"));
                                  is_valid = {status_codes::BadRequest, error};
                              }

                              is_valid = ValidateMsg(msg);

                              if (is_valid.code != status_codes::OK)
                              {
                                  request.reply(is_valid.code, is_valid.error);
                                  return;
                              }

                              auto msg_utf = msg.c_str();
                              auto msg_length = msg.length();

                              auto secret = config_.GetSecret();

                              auto hmac = Hmac(secret);

                              auto sig = hmac.Sign(msg_utf, msg_length);

                              json::value response;
                              response[U("signature")] = json::value::string(sig);

                              request.reply(status_codes::OK, response); })
                    .wait();
                {
                    LOG_INFO(Logger(config_.GetLogFile()), "Sign handled successfully");
                }
            }
            catch (const std::exception &e)
            {
                {
                    LOG_DEBUG(Logger(config_.GetLogFile()), "Exception in sign handler: " + std::string(e.what()));
                }

                json::value error;
                error[U("error")] = json::value::string(U("internal"));

                request.reply(status_codes::InternalError, error);
            }
        }

        void handle_verify(http_request request)
        {
            if (!request.headers().has(U("Content-Type")) ||
                request.headers()[U("Content-Type")].find(U("application/json")) == utility::string_t::npos)
            {
                json::value error;
                error[U("error")] = json::value::string(U("invalid_json"));
                request.reply(status_codes::UnsupportedMediaType,
                              error);
                return;
            }

            try
            {
                request.extract_json()
                    .then([&](json::value body)
                          {

                if (body.is_null()) {
                    request.reply(status_codes::BadRequest, 
                                json::value::string(U("invalid_msg")));
                    return;
                }
                
                if (!body.has_field(U("msg")) || !body.has_field(U("signature"))) {
                    json::value error;
                    error[U("error")] = json::value::string(U("invalid_msg"));
                    request.reply(status_codes::BadRequest, error);
                    return;
                }
        

                auto msg = body.at(U("msg")).as_string();
                auto sig = body.at(U("signature")).as_string();

                auto is_valid = ValidateMsg(msg);

                if (is_valid.code != status_codes::OK) {
                    request.reply(is_valid.code, is_valid.error);
                    return;
                }

                is_valid = ValidateMsg(sig);

                if (is_valid.code != status_codes::OK) {
                    request.reply(is_valid.code, is_valid.error);
                    return;
                }

                if (!isValidBase64Url(sig)) {
                    json::value error;
                    error[U("error")] = json::value::string(U("invalid_signature_format"));
                    is_valid = {status_codes::BadRequest, error};
                }

                if (is_valid.code != status_codes::OK) {
                    request.reply(is_valid.code, is_valid.error);
                    return;
                }
                
                auto secret = config_.GetSecret();

                auto hmac = Hmac(secret);

                auto is_ok = hmac.Verify(msg.c_str(), std::move(sig), msg.length());

                json::value response;
                response[U("ok")] = json::value::boolean(is_ok);
                
                request.reply(status_codes::OK, response); })
                    .wait();
                {
                    LOG_INFO(Logger(config_.GetLogFile()), "Verify handled successfully");
                }
            }
            catch (const json::json_exception &e)
            {
                request.reply(status_codes::UnsupportedMediaType, json::value::string(U("invalid_json")));
                return;
            }
            catch (const std::exception &e)
            {
                {
                    LOG_DEBUG(Logger(config_.GetLogFile()), "Exception in sign handler: " + std::string(e.what()));
                }

                json::value error;
                error[U("error")] = json::value::string(U("internal"));
                request.reply(status_codes::InternalError, error);
            }
        }
    };

}