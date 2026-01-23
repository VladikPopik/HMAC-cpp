// config.hpp
#pragma once
#include <memory>
#include <string>
#include <fstream>

namespace service::config {

class Config {
public:
    explicit Config(std::fstream& fs);
    
    ~Config();
    
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    Config(Config&&) noexcept;
    Config& operator=(Config&&) noexcept;
    
    void ReadStream();
    std::string GetAlg() const;
    std::string GetListen() const;
    std::string GetLogLevel() const;
    uint32_t GetMaxSizeBytes() const;
    std::string GetSecret() const;

private:
    struct Impl;
    
    std::unique_ptr<Impl> pImpl;
};

} // namespace service::config