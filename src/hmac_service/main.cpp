#include "config.hpp"
#include "http.hpp"
#include "logging.hpp"
#include <filesystem>
#include <fstream>

using namespace service::micro;
using namespace service::logging;

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    std::cerr << "Invalid args, please specify config file path" << std::endl;
    return 1;
  }

  std::string str_path = argv[1];

  std::filesystem::path fpath(str_path);

  fpath = std::filesystem::absolute(fpath);

  Config config(fpath);

  {
    LOG_INFO(Logger(config.GetLogFile()), "Starting service...");
  }

  Service service(std::move(config), fpath);

  service.start();

  try
  {
    while (true)
    {
    }
  }
  catch (std::exception &e)
  {
    service.stop();
  }

  return 0;
}