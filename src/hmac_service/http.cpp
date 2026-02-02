#include "http.hpp"
#include "logging.hpp"
#include <iostream>

namespace service::micro {

    using namespace service::logging;

    void Service::start() {
        auto logger = Logger(config_.GetLogLevel());
         try {
            listener_ping_.open().wait();
            listener_settings_.open().wait();
            listener_sign_.open().wait();
            listener_verify_.open().wait();
            logger.info("Service started " + listener_ping_.uri().to_string());
        } catch (const std::exception& e) {
            logger.error("Error while starting service");
            logger.error(e.what());
        }
    }

    void Service::stop() {
        auto logger = Logger(config_.GetLogLevel());
        listener_ping_.close().wait();
        listener_settings_.close().wait();
        listener_sign_.close().wait();
        listener_verify_.close().wait();
        logger.info("Microservice stopped");
    }
}