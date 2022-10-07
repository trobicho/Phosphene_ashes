#include "sceneLoader.hpp"
#include <iostream>

void  SceneLoader::test(const std::string &filename) {
  std::ifstream file(filename);
  json data = json::parse(file);

  std::cout << std::endl << "----------------JSON--------------" << std::endl;
  std::cout << data.dump(4) << std::endl;
  std::cout << "----------------------------------" << std::endl;
}
