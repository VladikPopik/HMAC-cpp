#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "rotate.hpp"

using namespace crypto;

int main(int argc, char *argv[]) {
  if (argc < 4) {
      std::cout << "Invalid args, please specify config file path" << "\n";
      return 1;
  }

  std::string fpath = argv[1];
  std::string new_secret_str = argv[2];
  int length = std::atoi(argv[3]);
  std::fstream fs(fpath);

  Rotate::UpdateConfig(fs, std::move(new_secret_str), length);

  std::cout << "Rotated config: " << fpath << " successfully" << std::endl; 
}