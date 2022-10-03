#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 outUV;
layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) uniform sampler2D inText;

void main() {
  vec2    uv = outUV;
  float   gamma = 1. / 2.2;
  fragColor = pow(texture(inText, uv).rgba, vec4(gamma));
}
