#include "rotate.hpp"

namespace crypto
{

    void Rotate::UpdateConfig(const std::string &fpath, uint64_t length)
    {

        SetupPermissions(fpath);

        {
            LOG_INFO(Logger("rotate.log"), "Permissions setup for owner");
        }

        std::fstream fs(fpath);

        json data_ = json::parse(fs);

        fs.close();

        auto secret = GenRandStr(length);

        data_["secret"] = secret;

        {
            LOG_DEBUG(Logger(log_file_), "New secret generated");
        }

        std::ofstream ofs(fpath);

        auto dump_data_ = data_.dump(4);

        ofs << dump_data_;

        ofs.close();
    }
}