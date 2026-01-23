#include "http.hpp"
#include <iostream>

namespace service::micro {
    void Service::start() {
         try {
            listener_.open().wait();
            std::cout << "Service started at: " << listener_.uri().to_string() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error starting service: " << e.what() << std::endl;
        }
    }

    void Service::stop() {
        listener_.close().wait();
        std::cout << "Microservice stopped" << std::endl;
    }
}