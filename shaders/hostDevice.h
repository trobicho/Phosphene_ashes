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
  eTextures = 1
END_BINDING();

START_BINDING(BindingsSceneOther) //Binding set 3
  eLights = 0
END_BINDING();

struct  GlobalUniforms {
  mat4  viewProj;
  mat4  viewInverse;
  mat4  projInverse;
};

// Push constant structure for the ray tracer
struct  PushConstantRay {
  vec4  clearColor;
  uint  nbLights;
};

struct  Vertex {
  vec3  pos;
  vec3  normal;
  vec3  color;
  vec2  textCoord;
};

struct  WaveFrontMaterial  // See ObjLoader, copy of MaterialObj, could be compressed for device
{
  vec3  ambient;
  vec3  diffuse;
  vec3  specular;
  vec3  transmittance;
  vec3  emission;
  float shininess;
  float ior;       // index of refraction
  float dissolve;  // 1 == opaque; 0 == fully transparent
  int   illum;     // illumination model (see http://www.fileformat.info/format/material/)
  int   textureId;
};

struct  Light {
  float intensity;
  vec3  pos;
  vec3  color;
};

struct  MeshDesc {
  int       textOffset;
  uint64_t  vertexAddress;
  uint64_t  indexAddress;
};

#endif
