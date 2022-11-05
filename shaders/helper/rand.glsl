float   randN = 1;

float   rand()
{
  vec2    co = gl_LaunchIDEXT.xy + randN + seed;

  float a = 12.9898;
  float b = 78.233;
  float c = 43758.5453;
  float dt = dot(co.xy, vec2(a,b));
  float sn = mod(dt, 3.1415);
  randN += 0.587;
  return fract(sin(sn) * c);
}

vec2    box_muller()
{
  float   r1 = rand();
  float   r2 = rand();
  float   tmp = sqrt(-2 * log(r1));
  vec2    d = vec2(tmp * cos(2 * M_PI * r2), tmp * sin(2 * M_PI * r2));

  return (d);
}

vec3    rand_dir(vec3 normal)
{
  float r1 = rand();
  float r2 = rand();

  float f = sqrt(1 - r2);
  vec3  dr_loc = vec3(cos(2 * M_PI * r1) * f, sin(2 * M_PI * r1) * f, sqrt(r2));
  vec3  vr = vec3(rand() - 0.5, rand() - 0.5, rand() - 0.5);
  vec3  tan1 = normalize(cross(normal, vr));
  vec3  tan2 = cross(tan1, normal);    return (dr_loc.z * normal + dr_loc.x * tan1 + dr_loc.y * tan2);
}
