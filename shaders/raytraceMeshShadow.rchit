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

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Positions of an object
layout(buffer_reference, scalar) buffer Indices {ivec3 i[]; }; // Triangle indices

layout(set = 0, binding = eTlas) uniform accelerationStructureEXT topLevelAS;
layout(set = 2, binding = eMeshDescs, scalar) buffer MeshDesc_ { MeshDesc i[]; } meshDescs;
layout(set = 2, binding = eMaterials, scalar) buffer Material_ { Material i[]; } materials;
layout(set = 3, binding = eLights, scalar) buffer Light_ { Light i[]; } lights;
hitAttributeEXT vec2 attribs;

void main()
{
  // Object data                                                                    
  MeshDesc  objResource = meshDescs.i[gl_InstanceCustomIndexEXT];
  Indices   indices     = Indices(objResource.indexAddress);
  Vertices  vertices    = Vertices(objResource.vertexAddress);
  Material  material    = materials.i[objResource.materialId];

  // Indices of the triangle                                                        
  ivec3 ind = indices.i[gl_PrimitiveID];

  // Vertex of the triangle
  Vertex v0 = vertices.v[ind.x];
  Vertex v1 = vertices.v[ind.y];
  Vertex v2 = vertices.v[ind.z];

  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  // Computing the coordinates of the hit position
  const vec3 pos = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
  const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space

  // Computing the normal at hit position
  const vec3 normal = v0.normal * barycentrics.x + v1.normal * barycentrics.y + v2.normal * barycentrics.z;
  const vec3 worldNrm = normalize(vec3(normal * gl_WorldToObjectEXT));  // Transforming the normal to world space


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
    vec3  origin = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
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
