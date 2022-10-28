float sdBox(in vec3 p, in vec3 b)
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float maxcomp(in vec2 v) {
  return (max(v.x, v.y));
}

float maxcomp(in vec3 v) {
  return (max(max(v.x, v.y), v.z));
}


mat4 rotation_matrix(in vec3 axis, const float angle)
{
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;
  axis = normalize(axis);

  return mat4(oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s,  0.0,
      oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0, 
      oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c, 0.0, 
      0.0, 0.0, 0.0, 1.0);
}
