// config.cpp
#include "config.hpp"
#include <nlohmann/json.hpp>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <utility>

namespace service::config {

class Config::Impl {
public:
    explicit Impl(std::fstream& fs) : fs_(fs) {
        ReadStream();
    }
    
    ~Impl() = default;
    
    Impl(const Impl&) = delete;
    Impl& operator=(const Impl&) = delete;
    
    Impl(Impl&&) noexcept = default;
    Impl& operator=(Impl&&) noexcept = delete;
    
    void ReadStream() {
        if (!fs_.is_open() && is_loaded_) {
            return;
        }

        json data_ = json::parse(fs_);
        body_ = ConfigBody(
            data_["hmac_alg"].get<std::string>(),
            data_["listen"].get<std::string>(),
            data_["log_level"].get<std::string>(),
            data_["max_msg_size_bytes"].get<uint32_t>(),
            base64_encode(data_["secret"].get<std::string>())
        );
        fs_.close();
    }
    
    std::string GetAlg() const { return body_.hmac_algo_; }
    std::string GetListen() const { return body_.listen_; }
    std::string GetLogLevel() const { return body_.log_level_; }
    uint32_t GetMaxSizeBytes() const { return body_.max_msg_size_bytes_; }
    std::string GetSecret() const { return body_.secret_; }

private:
    struct ConfigBody {
        std::string hmac_algo_;
        std::string listen_;
        std::string log_level_;
        uint32_t max_msg_size_bytes_;
        std::string secret_;

        explicit ConfigBody(std::string hmac, std::string listen, 
                           std::string log_level, uint32_t max_msg_size_bytes, 
                           std::string secret) 
            : hmac_algo_(std::move(hmac))
            , listen_(std::move(listen))
            , log_level_(std::move(log_level))
            , max_msg_size_bytes_(max_msg_size_bytes)
            , secret_(std::move(secret)) {}
    };
    
    using json = nlohmann::json;
    
    ConfigBody body_{{}, {}, {}, {}, {}};
    bool is_loaded_ = false;
    std::fstream& fs_;
    
    static std::string base64_encode(const std::string& input) {
        if (input.empty()) return {};
        
        BIO* b64 = BIO_new(BIO_f_base64());
        BIO* mem = BIO_new(BIO_s_mem());
        
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        BIO_push(b64, mem);
        
        BIO_write(b64, input.data(), static_cast<int>(input.length()));
        BIO_flush(b64);
        
        BUF_MEM* bufferPtr;
        BIO_get_mem_ptr(mem, &bufferPtr);
        
        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(b64);
        
        return result;
    }
};

Config::Config(std::fstream& fs) : pImpl(std::make_unique<Impl>(fs)) {}

Config::~Config() = default;

Config::Config(Config&& other) noexcept = default;

Config& Config::operator=(Config&& other) noexcept = default;

void Config::ReadStream() { pImpl->ReadStream(); }
std::string Config::GetAlg() const { return pImpl->GetAlg(); }
std::string Config::GetListen() const { return pImpl->GetListen(); }
std::string Config::GetLogLevel() const { return pImpl->GetLogLevel(); }
uint32_t Config::GetMaxSizeBytes() const { return pImpl->GetMaxSizeBytes(); }
std::string Config::GetSecret() const { return pImpl->GetSecret(); }

} // namespace service::config