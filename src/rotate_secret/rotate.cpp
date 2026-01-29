#include "rotate.hpp"

namespace crypto {


    void Rotate::UpdateConfig(const std::string& fpath, uint64_t length) {
        Rotate::SetupPermissions(fpath, Mode::READ_WRITE);

        std::fstream fs(fpath);

        json data_ = json::parse(fs);

        fs.close();

        std::cout << "Started generation of new secret" << "\n";

        auto secret = Rotate::GenRandStr(length);

        std::cout << "Secret generated" << "\n";

        data_["secret"] = secret;

        std::ofstream ofs(fpath);

        std::cout << data_ << std::endl;

        auto dump_data_ = data_.dump(4);

        ofs << dump_data_;

        ofs.close();

        Rotate::SetupPermissions(fpath, Mode::READ);

        std::cout << "Inserted" << "\n";

    }

}