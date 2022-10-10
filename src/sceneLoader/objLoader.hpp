#pragma once
#include "scene.hpp"

namespace ObjLoader {

struct  ObjLoaderConfig {
  bool        loadMaterial = true;
  bool        invertY = false;
  float       scale = 1.0;
  std::string scenePath;
  bool        useRelativePath = false;
};

void  load(const std::string filename, PhosObjectMesh &mesh, const ObjLoaderConfig &config);

}
