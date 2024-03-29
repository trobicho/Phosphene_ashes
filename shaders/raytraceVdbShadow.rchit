#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "hostDevice.h"
#include "raycommon.glsl"
#include "helper/illumination.glsl"

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;

layout(set = 0, binding = eTlas) uniform accelerationStructureEXT topLevelAS;
layout(set = 3, binding = eVdbDescs, scalar) buffer VdbDesc_ { VdbDesc i[]; } vdbDescs;
layout(set = 3, binding = eMaterials, scalar) buffer Material_ { Material i[]; } materials;
layout(set = 4, binding = eLights, scalar) buffer Light_ { Light i[]; } lights;

hitAttributeEXT block {
  vec3  pos;
  vec3  normal;
  uint  step;
  vec3  color;
}attribs;

void main()
{
  VdbDesc		vdb = vdbDescs.i[gl_InstanceCustomIndexEXT];
  Material  material = materials.i[vdb.materialId];

  const vec3 worldNrm = normalize(vec3(attribs.normal * gl_ObjectToWorldEXT));  // Transforming the normal to world space
  //const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(attribs.pos, 1.0));
  const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(attribs.pos, 1.0));

  float attenuation = 1;

  // Tracing shadow ray only if the light is visible from the surface
  vec3  lightDir = lights.i[0].pos - worldPos;
  float lightDistance = length(lightDir);
  lightDir = normalize(lightDir);

  vec3  specular = vec3(0.0);
  vec3  diffuse = vec3(0, 0, 0);
  float dl2 = lightDistance * lightDistance;
  if(dot(worldNrm, lightDir) > 0)
  {
    float tMin   = 0.001;
    float tMax   = lightDistance;
    vec3  origin = worldPos;
    vec3  rayDir = lightDir;
    uint  flags =
      gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    isShadowed = true;
    traceRayEXT(topLevelAS,  // acceleration structure
      flags,       // rayFlags
      0xFF,        // cullMask
      0,           // sbtRecordOffset
      0,           // sbtRecordStride
      1,           // missIndex
      origin,      // ray origin
      tMin,        // ray min range
      rayDir,      // ray direction
      tMax,        // ray max range
      1            // payload (location = 1)
    );
    diffuse = computeDiffuse(material, lightDir, worldNrm);

    if(isShadowed)
      attenuation = 0.0;
    else
      specular = computePhong(material,  gl_WorldRayDirectionEXT, lightDir, worldNrm);
  }
  prd.hitValue = (diffuse + specular) * (lights.i[0].intensity / dl2) * attenuation;
}
