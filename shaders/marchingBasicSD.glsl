float sdBox(vec3 p, vec3 b)
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

float maxcomp(vec2 v) {
  return (max(v.x, v.y));
}

float maxcomp(vec3 v) {
  return (max(max(v.x, v.y), v.z));
}
