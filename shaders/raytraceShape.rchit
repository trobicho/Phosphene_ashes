#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable
#extension GL_GOOGLE_include_directive : enable

#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_buffer_reference2 : require

#include "hostDevice.h"
#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT vec3 hitValue;

hitAttributeEXT block {
  vec3  pos;
  vec3  normal;
  uint  step;
}attribs;

void main()
{
  hitValue = vec3(attribs.normal); 
}
