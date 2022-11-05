vec3    randPhong(vec3 r, float spec)
{
    float   r1 = rand();
    float   r2 = rand();

    float   sf = sqrt(1 - pow(r2, 2. / (spec + 1)));
    vec3    dr_loc = vec3(cos(2 * M_PI * r1) * sf, sin(2 * M_PI * r1) * sf, sqrt(pow(r2, 1. / (spec + 1))));
    vec3    vr = vec3(rand() - 0.5, rand() - 0.5, rand() - 0.5);
    vec3    tan1 = normalize(cross(r, vr));
    vec3    tan2 = cross(tan1, r);
    return (dr_loc.z * r + dr_loc.x * tan1 + dr_loc.y * tan2);
}
