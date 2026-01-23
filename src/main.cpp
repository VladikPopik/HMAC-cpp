#include <iostream>
#include <fstream>
#include "hmac_service.hpp"
#include "config.hpp"

using namespace service::config;
using namespace service::hmac;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Invalid args, please specify config file path" << "\n";
        return 1;
    }

    std::string fpath = argv[1];
    std::fstream fs(fpath);

    Config config(fs);

    // EncodeResult test = Codec::Encode("test", config.GetSecret());
    // std::string test_decode = Codec::ToBase64Url(test.sig, test.len);

    // EncodeResult test2 = Codec::Encode("test", config.GetSecret());
    // std::string test2_decode = Codec::ToBase64Url(test2.sig, test2.len);

    std::string test = HmacService::Sign("test", config.GetSecret());
    std::string test2 = HmacService::Sign("test", config.GetSecret());

    std::cout << test <<"\n";
    std::cout << test2 << "\n";

    std::cout << HmacService::Verify(std::move(test2), std::move(test)) << "\n";

    std::cout << "Alg: " << config.GetAlg() << "\n";
    std::cout << "Listen: " << config.GetListen() << "\n";
    std::cout << "Log Level: " << config.GetLogLevel() << "\n";
    std::cout << "Max Size: " << config.GetMaxSizeBytes() << "\n";
    std::cout << "Secret: " << config.GetSecret() << "\n";

    return 0;
}