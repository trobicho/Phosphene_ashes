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

    void  test(const std::string &filename);

  private:
    PhosScene&  m_scene;
};
