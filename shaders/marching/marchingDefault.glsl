#include "../hostDevice.h"

layout(set = 2, binding = eShapeDescs, scalar) buffer ShapeDesc_ { ShapeDesc i[]; } shapeDescs;

hitAttributeEXT block {
  vec3  pos;
  vec3  normal;
  uint  step;
  vec3  color;
}attribs;

void  phosDefaultEntryPoint()
{
  ShapeDesc shape = shapeDescs.i[gl_InstanceCustomIndexEXT];

  Ray ray = {gl_ObjectRayOriginEXT, normalize(gl_ObjectRayDirectionEXT)};

  float maxDist = length(shape.aabb.max - shape.aabb.min) + 0.1f;
  float minDist = shape.marchingMinDist;
  float   dist = hitAabb(shape.aabb, ray);
  float   distT = 0;
  if (dist > 0) {
    ray.origin = ray.origin + ray.direction * dist;
    distT = dist;
  }

  Hit hit = marching(ray, minDist, maxDist, shape.marchingMaxStep);
  hit.dist += distT;
  if (hit.asHit) {
    attribs.pos = hit.pos;
    attribs.step = hit.step;
    if ((gl_IncomingRayFlagsEXT & gl_RayFlagsSkipClosestHitShaderEXT) == 0) {
      Ray   rayNormal = {hit.pos, ray.direction};
      vec3  normal = getNormal(rayNormal, minDist);
      attribs.normal = normal;
      attribs.color = getColor(hit);
    }

    float reportDist = abs(hit.dist) / length(gl_ObjectRayDirectionEXT);
    if (reportDist < gl_RayTminEXT)
      reportDist = gl_RayTminEXT;
    reportIntersectionEXT(reportDist, 1);
  }
}
