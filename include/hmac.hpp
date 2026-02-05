#pragma once

#include "codec.hpp"
#include <iostream>
#include <openssl/crypto.h>
#include <string>

namespace service::hmac
{

  using namespace service::codec;

  struct EncodeResult
  {
    const unsigned char *sig;
    unsigned int len;
  };

  class Hmac
  {

  public:
    explicit Hmac(const std::string &secret) : secret_(secret) {}

    std::string Sign(const char *msg, size_t length) const
    {
      EncodeResult encoded = HmacEncode(msg, length);
      return Codec::ToBase64Url(encoded.sig, encoded.len);
    }

    bool Verify(const char *msg, std::string &&sig, size_t length) const
    {

      std::string hash_msg(Sign(std::move(msg), length));

      if (hash_msg.length() != sig.length())
      {
        return false;
      }

      const char *data = hash_msg.c_str();
      const char *key = sig.c_str();

      return CRYPTO_memcmp(data, key, hash_msg.length()) == 0;
    }

  private:
    std::string secret_;

    EncodeResult HmacEncode(const char *data, size_t length) const
    {
      unsigned int len = 0;
      unsigned char out[EVP_MAX_MD_SIZE];

      return {HMAC(EVP_sha256(), secret_.c_str(), static_cast<int>(secret_.length()),
                   reinterpret_cast<const unsigned char *>(data),
                   static_cast<int>(length), out, &len),
              len};
    }
  };

} // namespace service::hmac