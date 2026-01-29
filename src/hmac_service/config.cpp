// config.cpp
#include "config.hpp"
#include "codec.hpp"
#include <nlohmann/json.hpp>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <utility>
namespace service::config {

using namespace service::codec;

class Config::Impl {
public:
  explicit Impl(const std::string &fpath) : config_path_(fpath) { ReadStream(); }

  ~Impl() = default;

  Impl(const Impl &) = delete;
  Impl &operator=(const Impl &) = delete;

  Impl(Impl &&) noexcept = default;
  Impl &operator=(Impl &&) noexcept = delete;

  void ReadStream() {
    std::fstream fs(config_path_);

    if (!fs.is_open()) {
      return;
    }

    json data_ = json::parse(fs);
    body_ = ConfigBody(data_["hmac_alg"].get<std::string>(),
                       data_["listen"].get<std::string>(),
                       data_["log_level"].get<std::string>(),
                       data_["max_msg_size_bytes"].get<uint32_t>(),
                       Codec::ToBase64Url(data_["secret"].get<std::string>()));
    fs.close();
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
        : hmac_algo_(std::move(hmac)), listen_(std::move(listen)),
          log_level_(std::move(log_level)),
          max_msg_size_bytes_(max_msg_size_bytes), secret_(std::move(secret)) {}
  };

  using json = nlohmann::json;

  ConfigBody body_{{}, {}, {}, {}, {}};
  std::string config_path_;
};

Config::Config(const std::string& fpath) : pImpl(std::make_unique<Impl>(fpath)) {}

Config::~Config() = default;

Config::Config(Config &&other) noexcept = default;

Config &Config::operator=(Config &&other) noexcept = default;

void Config::ReadStream() { pImpl->ReadStream(); }
std::string Config::GetAlg() const { return pImpl->GetAlg(); }
std::string Config::GetListen() const { return pImpl->GetListen(); }
std::string Config::GetLogLevel() const { return pImpl->GetLogLevel(); }
uint32_t Config::GetMaxSizeBytes() const { return pImpl->GetMaxSizeBytes(); }
std::string Config::GetSecret() const { return pImpl->GetSecret(); }

} // namespace service::config