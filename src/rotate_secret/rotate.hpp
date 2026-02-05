#pragma once
#include "hmac.hpp"
#include "logging.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <ostream>
#include <random>

namespace crypto
{

    using namespace service::hmac;
    using namespace service::logging;

    namespace fs = std::filesystem;

    class Rotate
    {
    public:
        Rotate(fs::path log_file) : log_file_(log_file) {};

        ~Rotate() = default;

        void UpdateConfig(const std::string &fpath, uint64_t length);

    private:
        using json = nlohmann::json;

        fs::path log_file_;

        void SetupPermissions(const std::string &fpath)
        {
            try
            {
                fs::permissions(fpath,
                                fs::perms::owner_read | fs::perms::owner_write,
                                fs::perm_options::replace);
            }
            catch (fs::filesystem_error const &e)
            {
                LOG_ERROR(Logger("rotate.log"), "Error changing permissions: ");
                LOG_ERROR(Logger("rotate.log"), e.what());
            }
        }

        std::string GenRandStr(size_t length)
        {
            const std::string characters =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

            std::random_device rd;
            std::mt19937 generator(rd());

            std::uniform_int_distribution<> distribution(0, characters.length() - 1);

            std::string random_string(length, ' ');
            std::generate_n(random_string.begin(), length, [&]()
                            { return characters[distribution(generator)]; });
            return random_string;
        }
    };

}