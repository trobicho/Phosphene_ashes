#include "sceneLoader.hpp"
#include "objLoader.hpp"
#include "vdbLoader.hpp"
#include <iostream>
#include <map>
#include <numeric>
#include <limits>

static bool  parseVec3(json& vecData, glm::vec3& vec) {
  if (vecData.is_array() && vecData.size() == 3) {
    vec.x = vecData[0];
    vec.y = vecData[1];
    vec.z = vecData[2];
    return (true);
  }
  else if (vecData.is_object()) {
    if (vecData["x"].is_number()
        && vecData["y"].is_number()
        && vecData["z"].is_number()) {
      vec.x = static_cast<float>(vecData["x"]);
      vec.y = static_cast<float>(vecData["y"]);
      vec.z = static_cast<float>(vecData["z"]);
      return (true);
    }
    if (vecData["r"].is_number()
        && vecData["g"].is_number()
        && vecData["b"].is_number()) {
      vec.x = static_cast<float>(vecData["r"]);
      vec.y = static_cast<float>(vecData["g"]);
      vec.z = static_cast<float>(vecData["b"]);
      return (true);
    }
  }
  return (false);
}

SceneLoader::SceneLoader(PhosScene& scene): m_scene(scene) {
}

void  SceneLoader::load(const std::string& filename) {
  std::ifstream file(filename);
  json data = json::parse(file);

  /*
  std::cout << std::endl << "----------------JSON--------------" << std::endl;
  std::cout << data.dump(4) << std::endl;
  std::cout << "----------------------------------" << std::endl;
  */

  m_scenePath = filename.substr(0, filename.find_last_of('/') + 1);

  for (auto &materialData : data["materials"]) {
    PhosMaterial  material;
    if (parseMaterial(materialData, material)) {
      m_scene.m_materials.push_back(material);
    }
  }
  for (auto &shaderData : data["shaders"]) {
    PhosHitShader   shader;
    if (parseShader(shaderData, shader))
      m_scene.m_hitShaders.push_back(shader);
  }
  for (auto &meshData : data["meshs"]) {
    PhosObjectMesh  mesh;
    if (parseMesh(meshData, mesh))
      m_scene.m_meshs.push_back(mesh);
  }
  for (auto &shapeData : data["shapes"]) {
    PhosObjectProcedural  shape;
    if (parseProceduralShape(shapeData, shape))
      m_scene.m_proceduralShapes.push_back(shape);
  }
  for (auto &vdbData : data["vdbs"]) {
    PhosObjectVdb	vdb;
    if (parseVdb(vdbData, vdb))
      m_scene.m_vdbs.push_back(vdb);
  }
  for (auto &instanceData : data["instances"]) {
    PhosObjectInstance  instance;
    if (parseInstance(instanceData, instance))
      m_scene.m_instances.push_back(instance);
  }
  for (auto &lightData : data["lights"]) {
    Light light;
    parseVec3(lightData["position"], light.pos);
    if (lightData["intensity"].is_number()) {
      light.intensity = static_cast<float>(lightData["intensity"]);
    }
    m_scene.m_lights.push_back(light);
  }
}

bool  SceneLoader::parseInstance(json& instanceData, PhosObjectInstance& instance) {
  instance.transform = glm::mat4(1.f);
  instance.objectType = 0;
  if (instanceData["mesh"].is_string()) {
    instance.objectName = instanceData["mesh"];
    instance.objectType = PHOS_OBJECT_TYPE_MESH;
  }
  else if (instanceData["shape"].is_string()) {
    instance.objectName = instanceData["shape"];
    instance.objectType = PHOS_OBJECT_TYPE_PROCEDURAL;
  }
  else if (instanceData["vdb"].is_string()) {
    instance.objectName = instanceData["vdb"];
    instance.objectType = PHOS_OBJECT_TYPE_VDB;
  }
  else {
    return (false);
  }
  if (instanceData["name"].is_string())
    instance.name = instanceData["name"];
  if (instanceData["material"].is_string())
    instance.materialName = instanceData["material"];
  else
    instance.materialName = PHOS_DEFAULT_MAT_NAME;
  if (instanceData["texture"].is_string())
    instance.textureName = instanceData["texture"];
  glm::vec3 v;
  if (parseVec3(instanceData["position"], v)) {
    instance.transform[0][3] = v.x;
    instance.transform[1][3] = v.y;
    instance.transform[2][3] = v.z;
  }
  if (instanceData["scale"].is_number()) {
    instance.transform[0][0] = static_cast<float>(instanceData["scale"]);
    instance.transform[1][1] = static_cast<float>(instanceData["scale"]);
    instance.transform[2][2] = static_cast<float>(instanceData["scale"]);
  }
  if (instanceData["marchingMaxStep"].is_number_integer())
    instance.marchingMaxStep = instanceData["marchingMaxStep"];
  if (instanceData["marchingMinDist"].is_number())
    instance.marchingMinDist = static_cast<float>(instanceData["marchingMinDist"]);

  if (instanceData["transform"].is_array()) {
    glm::mat4 matrix(1.0f);
    for (uint32_t r = 0; r < 3; r++) {
      for (uint32_t c = 0; c < instanceData["transform"][r].size(); c++) {
        matrix[r][c] = instanceData["transform"][r][c];
      }
    }
    instance.transform = matrix * instance.transform;
  }
  return (true);
}

bool  SceneLoader::parseMesh(json& meshData, PhosObjectMesh& mesh) {
  if (meshData["type"] == "wavefront") {
    ObjLoader::ObjLoaderConfig  config = {
      .scenePath = m_scenePath,
      .useRelativePath = true,
    };
    if (meshData["scale"].is_number())
      config.scale = static_cast<float>(meshData["scale"]);
    if (meshData["name"].is_string())
      mesh.name = meshData["name"];
    if (meshData["filepath"].is_string()) {
      std::string filepath = meshData["filepath"];
      if (filepath[0] == '/' || filepath[0] == '~')
        config.useRelativePath = false;
      ObjLoader::load(filepath, mesh, config);
    }
    return (true);
  }
  return (false);
}

bool  SceneLoader::parseVdb(json& vdbData, PhosObjectVdb& vdb) {
	VdbLoader::VdbLoaderConfig  config = {
		.scenePath = m_scenePath,
		.useRelativePath = true,
	};
	if (vdbData["scale"].is_number())
		config.scale = static_cast<float>(vdbData["scale"]);
	if (vdbData["name"].is_string())
		vdb.name = vdbData["name"];
	if (vdbData["grids"].is_array()) {
		for (auto &gridData: vdbData["grids"]) {
			if (gridData.is_string()) {
				PhosObjectVdb::VdbGrid grid;
				grid.name = gridData;
				vdb.grids.push_back(grid);
			}
		}
	}
	if (vdbData["filepath"].is_string()) {
		std::string filepath = vdbData["filepath"];
		if (filepath[0] == '/' || filepath[0] == '~')
			config.useRelativePath = false;
		VdbLoader::load(filepath, vdb, config);
	}
  if (vdbData["aabb"].is_object()) {
    glm::vec3 min, max;
		if (parseVec3(vdbData["aabb"]["min"], min) && parseVec3(vdbData["aabb"]["max"], max)) {
			vdb.aabb = {
        .minX = min.x,
        .minY = min.y,
        .minZ = min.z,
        .maxX = max.x,
        .maxY = max.y,
        .maxZ = max.z,
      };
    }
		else
			return (false);
  }
  return (true);
}

bool  SceneLoader::parseProceduralShape(json& shapeData, PhosObjectProcedural& shape) {
  if (shapeData["name"].is_string())
    shape.name = shapeData["name"];
  if (shapeData["aabb"].is_object()) {
    glm::vec3 min, max;
    if (parseVec3(shapeData["aabb"]["min"], min) && parseVec3(shapeData["aabb"]["max"], max)) {
      shape.aabb = {
        .minX = min.x,
        .minY = min.y,
        .minZ = min.z,
        .maxX = max.x,
        .maxY = max.y,
        .maxZ = max.z,
      };
    }
    else
      return (false);
  }
  if (shapeData["intersection"].is_string())
    shape.intersectionShaderName = shapeData["intersection"];
  else
    return (false);
  return (shape.intersectionShaderName != "");
}

bool  SceneLoader::parseShader(json& shaderData, PhosHitShader& shader) {
  if (shaderData["name"].is_string())
    shader.name = shaderData["name"];
  else
    return (false);
  if (shaderData["type"].is_string()) {
    if (shaderData["type"] == "intersection"
        || shaderData["type"] == "rint") {
      shader.type = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    }
    else
      return (false);
    if (shaderData["pName"].is_string())
      shader.pName = shaderData["pName"];
    else
      shader.pName = PHOS_DEFAULT_RINT_ENTRY_POINT;
  }
  else
    return (false);

  if (shaderData["spv"].is_string()) {
    std::string spvName = shaderData["spv"];
    shader.spv = spvName;
  }
  else
    return (false);
  return (true);
}

bool  SceneLoader::parseMaterial(json& materialData, PhosMaterial& material) {
  glm::vec3 v;

  material.diffuse = glm::vec3(1.0);
  material.specular = 0.0;
  material.transmittance = glm::vec3(0.5);
  material.emission = glm::vec3(0.0);
  material.refractionIndex = 0.0;
  material.shininess = 0.0;
  material.dissolve = 0.0;
  material.intensity = 0.0;

  if (materialData["name"].is_string())
    material.name = materialData["name"];
  else
    return (false);
  if (parseVec3(materialData["diffuse"], v))
    material.diffuse = v;
  if (parseVec3(materialData["transmittance"], v))
    material.transmittance = v;
  if (parseVec3(materialData["emission"], v))
    material.emission = v;

  if (materialData["refractionIndex"].is_number())
    material.refractionIndex = static_cast<float>(materialData["refractionIndex"]);
  if (materialData["shininess"].is_number())
    material.shininess = static_cast<float>(materialData["shininess"]);
  if (materialData["dissolve"].is_number())
    material.dissolve = static_cast<float>(materialData["dissolve"]);
  if (materialData["intensity"].is_number())
    material.intensity = static_cast<float>(materialData["intensity"]);
  if (materialData["specular"].is_number())
    material.specular = static_cast<float>(materialData["specular"]);

  return (true);
}

bool  SceneLoader::parseTexture(json& textureData, PhosTexture& texture) {
  return (true);
}
