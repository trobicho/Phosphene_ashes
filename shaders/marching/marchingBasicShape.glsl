float   plane_de(in vec3 p, const vec3 normal)
{
    return (length(p * normal.xyz) - 0.1);
}

float   cylinder_de(in vec3 p, const float radius)
{
    p.y = 0.0;
    return (length(p) - radius);
}

float   torus_de(in vec3 p, const float r1, const float r2)
{
    vec2 q = vec2(length(p.xz) - r1 , p.y);
    return length(q) - r2;
}

float   tetrahedron(in vec3 p)
{
    float r = max(max(-p.x - p.y - p.z, p.x + p.y - p.z), max(-p.x + p.y + p.z, p.x - p.y + p.z));
    return ((r - 1.0) / 1.73205);
}
