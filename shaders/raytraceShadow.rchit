#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

#include "hostDevice.h"
#include "raycommon.glsl"

layout(set = 0, binding = eTlas) uniform accelerationStructureEXT topLevelAS;
layout(set = 3, binding = eLights) uniform _Light { Light lights; };

layout(location = 0) rayPayloadInEXT hitPayload prd;
layout(location = 1) rayPayloadEXT bool isShadowed;
hitAttributeEXT vec3 attribs;

void main()
{
  float attenuation = 1;

  // Tracing shadow ray only if the light is visible from the surface
  vec3  origin = attribs.xyz;
  vec3  lightDir = lights.pos - origin;
  float lightDistance = length(lightDir);
  lightDir = normalize(lightDir);
  vec3  normal = vec3(0, 0, -1);
  normal = normalize(normal);

  float specular = 0.0;
  vec3  diffuse = vec3(0.1, 0, 0);
  if(dot(normal, lightDir) > 0)
  {
    float tMin   = 0.001;
    float tMax   = lightDistance;
    vec3  rayDir = lightDir;
    uint  flags =
      gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    isShadowed = true;
    origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
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
    float dotNL = max(dot(normal, lightDir), 0.0);
    diffuse = vec3(0.3 * dotNL);

    if(isShadowed)
    {
      attenuation = 0.3;
    }
    else
    {
      // Specular
      const float kPi        = 3.14159265;
      const float kShininess = 4.0;

      const float kEnergyConservation = (2.0 + kShininess) / (2.0 * kPi);
      vec3  V = normalize(-gl_WorldRayDirectionEXT);
      vec3  R = reflect(-lightDir, normal);
      specular = kEnergyConservation * pow(max(dot(V, R), 0.0), kShininess);
    }
  }
  prd.hitValue = vec3(lights.intensity * attenuation * (diffuse + specular));
  if (length(prd.hitValue.xyz) < 0.1)
    prd.hitValue = vec3(0.2);
}
