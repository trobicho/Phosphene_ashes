#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "hostDevice.h"
#include "pathcommon.glsl"

layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Positions of an object
layout(buffer_reference, scalar) buffer Indices {ivec3 i[]; }; // Triangle indices

layout(set = 2, binding = eMeshDescs, scalar) buffer MeshDesc_ { MeshDesc i[]; } meshDescs;
layout(set = 2, binding = eMaterials, scalar) buffer Material_ { Material i[]; } materials;
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


  prd.asHit = true;
  prd.matId = objResource.materialId;
  prd.normal = worldNrm;
  prd.hitPos = worldPos + worldNrm * 0.0001;
  prd.color = vec3(0.4f, 0.8f, 1.0f);
  prd.color = vec3(1.0f);
}
