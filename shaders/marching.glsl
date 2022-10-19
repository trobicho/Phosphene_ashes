#extension GL_EXT_ray_tracing : require

#define MAX_DIST    100 
#define MIN_DIST    0.001
#define MAX_STEP    1000

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

Hit marching(Ray ray)
{
  float   d = 0.;
  float   ds = 0.;
  vec3    p;

  for(int i = 0; i < MAX_STEP; i++)
  {
    p = ray.origin + (ray.direction * d);
    ds = sdf(p);
    d += ds;
    if (d > MAX_DIST) {
      return (Hit(false, vec3(0.f), d, MAX_STEP));
    }
    if (ds <= MIN_DIST) {
      Hit hit = {
        true,
        ray.origin,
        d,
        i
      };
      return (hit);
    }
  }
  return (Hit(false, vec3(0.f), d, MAX_STEP));
}
