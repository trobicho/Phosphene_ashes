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

struct  GlobalUniforms {
  mat4  viewProj;
  mat4  viewInverse;
  mat4  projInverse;
};

// Push constant structure for the ray tracer
struct  PushConstantRay {
  vec4  clearColor;
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
  vec3  pos;
  vec3  color;
  float intensity;
};

#endif
