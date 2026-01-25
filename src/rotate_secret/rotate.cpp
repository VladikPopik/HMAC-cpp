#include "rotate.hpp"

namespace crypto {

    void Rotate::UpdateConfig(const std::string& fpath, uint64_t length) {
        std::fstream fs(fpath);
        if (!fs.is_open()) {
            std::cerr << "Couldn't open file" << "\n";
            return;
        }
        json data_ = json::parse(fs);
        fs.close();
        std::cout << "Started generation of new secret" << "\n";

        auto secret = Rotate::GenRandStr(length);

        std::cout << "Secret generated" << "\n";

        data_["secret"] = secret;

        std::fstream ofs(fpath, std::ios_base::out);

        std::cout << data_ << std::endl;

        fs << data_.dump(4) << std::endl;

        std::cout << "Inserted" << "\n";

    }

}