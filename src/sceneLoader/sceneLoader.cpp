#include "sceneLoader.hpp"
#include "objLoader.hpp"
#include <iostream>
#include <map>
#include <numeric>
#include <limits>

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
      if (meshData["invertY"].is_boolean())
        config.invertY = meshData["invertY"];
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
  for (auto &instanceData : data["instances"]) {
    PhosObjectInstance instance;
    instance.transform = glm::mat4(1.f);
    if (instanceData["name"].is_string())
      instance.name = instanceData["name"];
    if (instanceData["mesh"].is_string()) {
      instance.objectName = instanceData["mesh"];
      instance.objectType = PHOS_OBJECT_TYPE_MESH;
    }
    if (instanceData["position"].is_array()) {
      instance.transform[0][3] = instanceData["position"][0];
      instance.transform[1][3] = instanceData["position"][1];
      instance.transform[2][3] = instanceData["position"][2];
    }
    if (instanceData["scale"].is_number_float()) {
      instance.transform[0][0] = instanceData["scale"];
      instance.transform[1][1] = instanceData["scale"];
      instance.transform[2][2] = instanceData["scale"];
    }
    m_scene.m_instances.push_back(instance);
  }
}
