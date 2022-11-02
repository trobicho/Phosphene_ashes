#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "hostDevice.h"

layout(location = 0) rayPayloadInEXT vec3 hitValue;
hitAttributeEXT vec2 attribs;

layout(buffer_reference, scalar) buffer Vertices {Vertex v[]; }; // Positions of an object
layout(buffer_reference, scalar) buffer Indices {ivec3 i[]; }; // Triangle indices

layout(set = 2, binding = eMeshDescs, scalar) buffer MeshDesc_ { MeshDesc i[]; } meshDesc;

void main()
{
  MeshDesc  objResource = meshDesc.i[gl_InstanceCustomIndexEXT];
  Indices   indices     = Indices(objResource.indexAddress);
  Vertices  vertices    = Vertices(objResource.vertexAddress);

  // Indices of the triangle                                                        
  ivec3 ind = indices.i[gl_PrimitiveID];

  // Vertex of the triangle
  Vertex v0 = vertices.v[ind.x];
  Vertex v1 = vertices.v[ind.y];
  Vertex v2 = vertices.v[ind.z];
  const vec3 worldNrm = abs(normalize(vec3(v0.normal * gl_WorldToObjectEXT)));  // Transforming the normal to world space
  hitValue = worldNrm;
}
