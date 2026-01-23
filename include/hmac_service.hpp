#pragma once
#include <string>
#include "codec.hpp"
#include <openssl/crypto.h>

namespace service::hmac {

    using namespace service::codec;

class HmacService {

public: 
    static std::string Sign(std::string&& msg, const std::string& secret) {
        EncodeResult encoded = Codec::Encode(msg, secret);
        return Codec::ToBase64Url(encoded.sig, encoded.len);
    }

    static bool Verify(std::string&& msg, std::string&& sig) {
        if (msg.length() != sig.length()) {
            return false;
        }
        
        const char * data = msg.c_str();
        const char * key = sig.c_str();

        return CRYPTO_memcmp(data, key, msg.length()) == 0;
    }

};

}