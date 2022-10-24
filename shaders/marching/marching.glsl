#extension GL_EXT_ray_tracing : require

struct  Ray {
  vec3  origin;
  vec3  direction;
};

struct  Hit {
  bool  asHit;
  vec3  pos;
  float dist;
  uint  step;
};

struct  Sphere {
  vec3  center;
  float radius;
};

float hitSphere(const Sphere s, const Ray r)
{
  vec3  oc           = r.origin - s.center;
  float a            = dot(r.direction, r.direction);
  float b            = 2.0 * dot(oc, r.direction);
  float c            = dot(oc, oc) - s.radius * s.radius;
  float discriminant = b * b - 4 * a * c;
  if(discriminant < 0)
  {
    return -1.0;
  }
  else
  {
    return (-b - sqrt(discriminant)) / (2.0 * a);
  }
}

Hit   marching(Ray ray, float minDist, uint maxStep)
{
  float   d = 0.;
  float   ds = 0.;
  vec3    p = ray.origin;

  for(int step = 0; step < maxStep; step++)
  {
    ds = sdf(p);
    d += ds;
    if (d > MAX_DIST) {
      return (Hit(false, vec3(0.f), d, step));
    }
    if (ds <= minDist) {
      Hit hit = {
        true,
        p,
        d,
        step
      };
      return (hit);
    }
    p = ray.origin + (ray.direction * d);
  }
  return (Hit(true, p, d, maxStep));
}

vec3  getNormal(Ray ray, float minDist)
{
    float   d;
    float   epsi;
    vec3    n;
    vec3    p;

    p = ray.origin;
    epsi = minDist / 2.0;
    d = sdf(p);
    n = vec3(d - sdf(vec3(p.x - epsi, p.yz))
        , d - sdf(vec3(p.x, p.y - epsi, p.z))
        , d - sdf(vec3(p.xy, p.z - epsi)));
    return (normalize(n));
}
