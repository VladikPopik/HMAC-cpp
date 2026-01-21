#pragma once
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <sstream>


namespace service::codec {

struct EncodeResult {
    unsigned char * sig;
    unsigned int len;
};

class Codec {
public:
    static EncodeResult Encode(std::string data, const std::string& secret) {
        unsigned int len=0;
        unsigned char out[EVP_MAX_MD_SIZE];

        return {HMAC(EVP_sha256(),
            secret.c_str(),
            static_cast<int>(data.length()),
            reinterpret_cast<const unsigned char*>(data.c_str()),
            static_cast<int>(data.size()), out, &len), len};
    }

    static std::string ToBase64Url(const unsigned char* sig, size_t len) {
        std::string out(4 * ((len + 2) / 3), '\0');
        int n = EVP_EncodeBlock(reinterpret_cast<unsigned char*>(&out[0]), sig, static_cast<int>(len));
        out.resize(n);
        for (char& c : out) { if (c == '+') c = '-'; else if (c == '/') c = '_'; }
        while (!out.empty() && out.back() == '=') out.pop_back();
        return out;
    }
};

} // namespace service::codec