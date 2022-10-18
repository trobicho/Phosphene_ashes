#pragma once
#include <fstream>
#include <string>
#include <json/json.hpp>
#include "scene.hpp"

using json = nlohmann::json;

class SceneLoader
{
  public:
    SceneLoader(PhosScene &scene); //Do I pass the reference here? probably not!

    void  load(const std::string &filename);

  private:
    bool  parseInstance(json &instanceData, PhosObjectInstance &instance);
    bool  parseMesh(json &meshData, PhosObjectMesh &mesh);
    bool  parseProceduralShape(json &shapeData, PhosObjectProcedural &shape);

    PhosScene&  m_scene;
    std::string m_scenePath;
};
