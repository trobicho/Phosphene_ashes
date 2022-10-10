#include "sceneLoader.hpp"
#include "objLoader.hpp"
#include <iostream>

SceneLoader::SceneLoader(PhosScene &scene): m_scene(scene) {
}

void  SceneLoader::test(const std::string &filename) {
  std::ifstream file(filename);
  json data = json::parse(file);

  std::cout << std::endl << "----------------JSON--------------" << std::endl;
  std::cout << data.dump(4) << std::endl;
  std::cout << "----------------------------------" << std::endl;

  std::string scenePath = filename.substr(0, filename.find_last_of('/') + 1);

  for (auto &meshData : data["meshs"]) {
    if (meshData["type"] == "wavefront") {
      PhosObjectMesh  mesh;
      ObjLoader::ObjLoaderConfig  config = {
        .scenePath = scenePath,
        .useRelativePath = true,
      };
      if (meshData["invertY"])
        config.invertY = true;
      if (meshData["scale"].is_number_float())
        config.scale = meshData["scale"];
      if (meshData["name"].is_string())
        mesh.m_name = meshData["name"];
      if (meshData["filepath"].is_string()) {
        std::string filepath = meshData["filepath"];
        ObjLoader::load(filepath, mesh, config);
      }
      m_scene.m_meshs.push_back(mesh);
    }
  }
}
