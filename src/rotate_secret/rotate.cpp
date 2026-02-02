#include "rotate.hpp"

namespace crypto {

    void Rotate::UpdateConfig(const std::string& fpath, uint64_t length) {
        Rotate::SetupPermissions(fpath);

        std::fstream fs(fpath);

        json data_ = json::parse(fs);

        fs.close();

        auto secret = Rotate::GenRandStr(length);

        data_["secret"] = secret;

        std::ofstream ofs(fpath);

        auto dump_data_ = data_.dump(4);

        ofs << dump_data_;

        ofs.close();
    }

}