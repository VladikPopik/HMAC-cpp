#include "http.hpp"
#include "logging.hpp"
#include <iostream>

namespace service::micro {

    using namespace service::logging;

    void Service::start() {
         try {
            listener_ping_.open().wait();
            listener_settings_.open().wait();
            listener_sign_.open().wait();
            listener_verify_.open().wait();
            {
                LOG_INFO(Logger(), "Service started " + listener_ping_.uri().to_string());
            }
        } catch (const std::exception& e) {

            {
                LOG_ERROR(Logger(), "Error while starting service");
                LOG_ERROR(Logger(), e.what());
            }
        }
    }

    void Service::stop() {
        listener_ping_.close().wait();
        listener_settings_.close().wait();
        listener_sign_.close().wait();
        listener_verify_.close().wait();
        {
            LOG_INFO(Logger(), "Microservice stopped");
        }
    }
}