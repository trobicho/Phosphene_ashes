#pragma once
#include "scene.hpp"

namespace VdbLoader {

struct  VdbLoaderConfig {
  bool        loadMaterial = true;
  float       scale = 1.0;
  std::string scenePath;
  bool        useRelativePath = false;
};

void	initialize();
void  load(const std::string filename, PhosObjectVdb &mesh, const VdbLoaderConfig &config);

}
