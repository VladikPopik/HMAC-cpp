#include "config.hpp"
#include "logging.hpp"
#include "http.hpp"
#include <fstream>

using namespace service::micro;
using namespace service::logging;

int main(int argc, char *argv[]) {
  if (argc < 2) {
      std::cerr << "Invalid args, please specify config file path" << "\n";
      return 1;
  }

  std::string fpath = argv[1];
  std::fstream fs(fpath);

  Service service(fs);

    service.start();

    try {
      while (true) { }
    } catch (std::exception& e) {
        service.stop();
    }

  return 0;
}