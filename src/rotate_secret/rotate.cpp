#include "rotate.hpp"

namespace crypto {

    void Rotate::UpdateConfig(std::fstream& fs, std::string&& new_secret_str, uint64_t length) {
        if (!fs.is_open()) {
            std::cerr << "Couldn't open file" << "\n";
        }

        std::cout << "Started generation of new secret" << "\n";

        auto secret = GenerateNewSecret(std::move(new_secret_str), length);

        std::cout << "Secret generated" << "\n";

        json data_ = json::parse(fs);

        data_["secret"] = secret;

        std::cout << data_ << std::endl;

        fs << data_ << std::endl;

        std::cout << "Inserted" << "\n";

    }

}