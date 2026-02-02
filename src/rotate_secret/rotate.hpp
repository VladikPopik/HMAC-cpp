#pragma once
#include <fstream>
#include <iostream>
#include "hmac.hpp"
#include <random>
#include <algorithm>
#include <nlohmann/json.hpp>
#include <ostream>
#include <filesystem>


namespace crypto {

using namespace service::hmac;

namespace fs = std::filesystem;


class Rotate {
public:
    static void UpdateConfig(const std::string& fpath, uint64_t length);

private:
    using json = nlohmann::json;


    static void SetupPermissions(const std::string& fpath) {
            try {
                fs::permissions(fpath, 
                                fs::perms::owner_read | fs::perms::owner_write,
                                fs::perm_options::replace);
            } catch (fs::filesystem_error const& ex) {
                std::cerr << "Error changing permissions: " << ex.what() << std::endl;
            }
    }


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
};

}