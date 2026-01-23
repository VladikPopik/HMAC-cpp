#include "config.hpp"
#include "http.hpp"
#include "hmac_service.hpp"
#include <fstream>
#include <iostream>

using namespace service::config;
using namespace service::hmac;
using namespace service::micro;

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Invalid args, please specify config file path" << "\n";
    return 1;
  }

  std::string fpath = argv[1];
  std::fstream fs(fpath);

  Config config(fs);

  HmacService hmac(config.GetSecret());

  std::string test = hmac.Sign("111");
  std::string test2 = hmac.Sign("11111");

  std::cout << hmac.Verify(std::move("test"), std::move(test)) << "\n";
  std::cout << hmac.Verify(std::move("11111"), std::move(test2)) << "\n";

  std::cout << "Alg: " << config.GetAlg() << "\n";
  std::cout << "Listen: " << config.GetListen() << "\n";
  std::cout << "Log Level: " << config.GetLogLevel() << "\n";
  std::cout << "Max Size: " << config.GetMaxSizeBytes() << "\n";
  std::cout << "Secret: " << config.GetSecret() << "\n";

    Service service(std::move(config.GetListen()));

    service.start();

    service.stop();

  return 0;
}