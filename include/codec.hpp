#pragma once
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <sstream>

namespace service::codec {



class Codec {
public:
  static std::string FromBase64Url(const std::string &input) {
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO *bio = BIO_new_mem_buf(input.c_str(), input.length());
    BIO_push(b64, bio);

    BUF_MEM *bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);

    BIO *mem_bio = BIO_new(BIO_s_mem());
    BIO_push(b64, mem_bio);

    char decode_buffer[1024];
    int decoded_len = 0;
    std::string output;

    while ((decoded_len = BIO_read(b64, decode_buffer, sizeof(decode_buffer))) >
           0) {
      output.append(decode_buffer, decoded_len);
    }

    BIO_free_all(b64);

    return output;
  }

  static std::string ToBase64Url(const unsigned char *sig, size_t len) {
    std::string out(4 * ((len + 2) / 3), '\0');
    int n = EVP_EncodeBlock(reinterpret_cast<unsigned char *>(&out[0]), sig,
                            static_cast<int>(len));
    out.resize(n);
    for (char &c : out) {
      if (c == '+')
        c = '-';
      else if (c == '/')
        c = '_';
    }
    while (!out.empty() && out.back() == '=')
      out.pop_back();
    return out;
  }

};

} // namespace service::codec