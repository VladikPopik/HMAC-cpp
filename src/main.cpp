#include <fstream>
#include <iostream>
#include "config.hpp"

using namespace service::config;

int main() {
    std::fstream fs("config.json");
    
    if (!fs.is_open()) {
        std::cerr << "Failed to open config.json" << std::endl;
        return 1;
    }

    Config config(fs);
    config.ReadStream();

    std::cout << "Alg: " << config.GetAlg() << "\n";
    std::cout << "Listen: " << config.GetListen() << "\n";
    std::cout << "Log Level: " << config.GetLogLevel() << "\n";
    std::cout << "Max Size: " << config.GetMaxSizeBytes() << "\n";
    std::cout << "Secret: " << config.GetSecret() << "\n";

    return 0;
}