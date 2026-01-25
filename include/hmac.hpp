#pragma once

#include "codec.hpp"
#include <iostream>
#include <openssl/crypto.h>
#include <string>

namespace service::hmac {

using namespace service::codec;

struct EncodeResult {
  const unsigned char *sig;
  unsigned int len;
};

class Hmac {

public:
  explicit Hmac(const std::string &secret) : secret_(secret) {}

  std::string Sign(std::string &&msg) const {
    EncodeResult encoded = HmacEncode(msg);
    return Codec::SigToBase64Url(encoded.sig, encoded.len);
  }

  bool Verify(std::string &&msg, std::string &&sig) const {

    if (msg.empty()) {
      return false;
    }

    std::string hash_msg(Sign(std::move(msg)));

    if (hash_msg.length() != sig.length()) {
      return false;
    }

    const char *data = hash_msg.c_str();
    const char *key = sig.c_str();

    return CRYPTO_memcmp(data, key, hash_msg.length()) == 0;
  }

private:
  std::string secret_;

  EncodeResult HmacEncode(std::string data) const {
    unsigned int len = 0;
    unsigned char out[EVP_MAX_MD_SIZE];

    return {HMAC(EVP_sha256(), secret_.c_str(), static_cast<int>(data.length()),
                 reinterpret_cast<const unsigned char *>(data.c_str()),
                 static_cast<int>(data.size()), out, &len),
            len};
  }
};

} // namespace service::hmac