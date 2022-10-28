#ifndef HOST_DEVICE_PICKER_HEADER
#define HOST_DEVICE_PICKER_HEADER

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

START_BINDING(BindingsPicker)
  ePickerTlas = 0,
  ePickerCamera = 1,
  ePickerResult= 2
END_BINDING();

struct  PushConstantPickRay {
  vec2  pick;
};

struct  PickerResult {
  vec3  worldRayOrigin;
  vec3  worldRayDirection;
  float hitT;
  uint  primitiveId;
  uint  instanceId;
  uint  instanceCustomIndex;
};

#endif
