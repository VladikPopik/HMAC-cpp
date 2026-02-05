#include "rotate.hpp"
#include <iostream>
#include <stdlib.h>

using namespace crypto;

int main(int argc, char *argv[])
{
  if (argc < 4)
  {
    std::cerr << "Invalid args, please specify config filepath, desired length of new secret and log_file where to write" << std::endl;
    return 1;
  }

  fs::path fpath = argv[1];
  int length = std::atoi(argv[2]);
  fs::path log_file = argv[3];

  {
    Logger log(log_file);
    LOG_INFO(log, "Starting rotation...");
  }

  Rotate rotate(log_file);
  rotate.UpdateConfig(std::move(fpath), length);

  {
    Logger log(log_file);
    LOG_INFO(log, "Rotated config successfully: ");
    LOG_INFO(log, fpath);
  }
}