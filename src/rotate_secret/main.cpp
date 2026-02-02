#include <stdlib.h>
#include "rotate.hpp"

using namespace crypto;

int main(int argc, char *argv[]) {
  if (argc < 3) {
      std::cerr << "\033[31m" << "Invalid args, please specify config filepath and desired length of new secret" << "\n";
      return 1;
  }

  fs::path fpath = argv[1];
  int length = std::atoi(argv[2]);

  Rotate::UpdateConfig(std::move(fpath), length);

  std::cout << "Rotated config: " << fpath << " successfully" << std::endl; 
}