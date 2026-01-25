#pragma once
#include <fstream>
#include <iostream>
#include "hmac.hpp"
#include "codec.hpp"
#include <random>
#include <algorithm>
#include <nlohmann/json.hpp>


namespace crypto {

using namespace service::hmac;

class Rotate {
public:
    static void UpdateConfig(std::fstream& fs, std::string&& new_secret_str, uint64_t length);

private:
    using json = nlohmann::json;


    static std::string GenRandStr(size_t length) {
        const std::string characters =
            "0123456789"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz";
        
        std::random_device rd;
        std::mt19937 generator(rd());
        
        std::uniform_int_distribution<> distribution(0, characters.length() - 1);
        
        std::string random_string(length, ' ');
        std::generate_n(random_string.begin(), length, [&]() {
            return characters[distribution(generator)];
        });
        return random_string;
    }

    static std::string GenerateNewSecret(std::string&& new_secret_str, uint64_t length) {
        Hmac hmac(new_secret_str);

        return hmac.Sign(std::move(GenRandStr(length)));
    }
};

}