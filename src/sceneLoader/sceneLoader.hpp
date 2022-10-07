#pragma once
#include <fstream>
#include <string>
#include <json/json.hpp>

using json = nlohmann::json;

class SceneLoader
{
  public:
    SceneLoader(){};

    void  test(const std::string &filename);
};
