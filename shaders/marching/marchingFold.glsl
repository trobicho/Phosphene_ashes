void box_fold(inout vec3 z, const float folding_limit) 
{
  z = clamp(z, -folding_limit, folding_limit) * 2.0 - z;
}

void sphere_fold(inout vec3 z, inout float dz, const float min_radius, const float fixed_radius) 
{
  float r = dot(z, z); 

  if (r < min_radius)
  { 
    float tmp = (fixed_radius / min_radius);
    z *= tmp;
    dz*= tmp;
  }
  else if (r < fixed_radius)
  { 
    float tmp = (fixed_radius / r);
    z *= tmp;
    dz *= tmp;
  }
}
