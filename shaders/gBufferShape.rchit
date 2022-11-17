#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "hostDevice.h"
#include "gBufferCommon.glsl"

layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 3, binding = eShapeDescs, scalar) buffer ShapeDesc_ { ShapeDesc i[]; } shapeDescs;
layout(set = 3, binding = eMaterials, scalar) buffer Material_ { Material i[]; } materials;

hitAttributeEXT block {
  vec3  pos;
  vec3  normal;
  uint  step;
  vec3  color;
}attribs;

void main()
{
  ShapeDesc shape = shapeDescs.i[gl_InstanceCustomIndexEXT];
  Material  material    = materials.i[shape.materialId];

  const vec3 worldNrm = (vec3(attribs.normal * gl_ObjectToWorldEXT));  // Transforming the normal to world space
  const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(attribs.pos + attribs.normal * shape.marchingMinDist * 2.5, 1.0));

  //prd.color = material.transmittance;
  prd.color = attribs.color;
  prd.normal = worldNrm;
  prd.depth = length(worldPos - gl_WorldRayOriginEXT);
  prd.objId = int(gl_InstanceCustomIndexEXT);
  prd.matId = int(shape.materialId);
  prd.asHit = true;
  prd.objId = -prd.objId;
}
