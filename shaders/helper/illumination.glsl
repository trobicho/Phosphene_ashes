#extension GL_GOOGLE_include_directive : enable

vec3 computeDiffuse(const Material mat, const vec3 lightDir, const vec3 normal)
{
  // Lambertian
  float dotNL = max(dot(normal, lightDir), 0.0);
  vec3  c     = mat.diffuse * dotNL;
  return c;
}

vec3 computePhong(const Material mat, const vec3 viewDir, const vec3 lightDir, const vec3 normal)
{
  // Compute specular only if not in shadow
  const float kPi = 3.14159265;
  const float kShininess = mat.shininess;

  // Specular
  const float kEnergyConservation = (2.0 + kShininess) / (2.0 * kPi);
  vec3        V                   = normalize(-viewDir);
  vec3        R                   = reflect(-lightDir, normal);
  float       specular            = kEnergyConservation * pow(max(dot(V, R), 0.0), kShininess);

  return (mat.specular * specular) * mat.transmittance;
}

vec3  computeCookTorr(const Material mat, const vec3 viewDir, const vec3 lightDir, const vec3 normal)
{
  const float kPi = 3.14159265;

  return(vec3(1.0f));
}
