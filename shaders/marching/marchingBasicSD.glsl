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
