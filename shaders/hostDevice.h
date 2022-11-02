#ifndef HOST_DEVICE_HEADER
#define HOST_DEVICE_HEADER

#ifdef __cplusplus
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat4 = glm::mat4;
using uint = unsigned int;
#endif

#ifdef __cplusplus
  #define START_BINDING(a) enum a {
  #define END_BINDING() }
#else
  #define START_BINDING(a) const uint
  #define END_BINDING()
#endif

START_BINDING(BindingsRtx) //Binding set 0
  eTlas = 0,
  eImageOut = 1
END_BINDING();

START_BINDING(BindingsCommon) //Binding set 1
  eGlobals = 0
END_BINDING();

START_BINDING(BindingsScene) //Binding set 2
  eMeshDescs = 0,
  eShapeDescs = 1,
  eTextures = 2,
  eMaterials = 3
END_BINDING();

START_BINDING(BindingsSceneOther) //Binding set 3
  eLights = 0
END_BINDING();

struct  GlobalUniforms {
  mat4  viewProj;
  mat4  viewInverse;
  mat4  projInverse;
  float time;
};

// Push constant structure for the ray tracer
struct  PushConstantRay {
  vec4  clearColor;
  uint  nbLights;
  uint  nbConsecutiveRay;
  uint  pathMaxRecursion;
};

struct  Vertex {
  vec3  pos;
  vec3  normal;
  vec3  color;
  vec2  textCoord;
};

struct  Material {
  vec3  ambient;
  vec3  diffuse;
  vec3  specular;
  vec3  transmittance;
  vec3  emission;

  float refractionIndex;
  float shininess;
  float dissolve; // 0.0 == opaque; 1.0 == fully transparent
  float intensity;
};

struct  Light {
  float intensity;
  vec3  pos;
  vec3  color;
};

struct  MeshDesc {
  int       textureId;
  uint      materialId;
  uint64_t  vertexAddress;
  uint64_t  indexAddress;
};

struct  Aabb {
  vec3  min;
  vec3  max;
};

struct  ShapeDesc {
  int   textureId;
  uint  materialId;
  float marchingMinDist;
  int   marchingMaxStep;
  Aabb  aabb;
};

#endif
