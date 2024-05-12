#include"util.hlsl"
bool RayIntersectScene(ray r,out MyTriangle outtri,out float2 b1b2)
{
    AABBbox box;
    int i = 0;
    while (i<boxNum)
    {
        box = g_boxData[i];
        if (RayIntersectAABBBox(box.center, box.extent, r))
        {
            if (box.isLeaf)
            {
                for (int j = 0; j < box.triangleNum; j++)
                {
                    MyTriangle t = g_triangleData[box.triangleStart + j];
                    if (RayIntersectTriangle(t.pos1, t.pos2, t.pos3, r,b1b2))
                    {
                        outtri = t;
                        return true;
                    }
                }
            }
            i++;
        }
        else
        {
            if (box.missIndex > 0)
            {
                i = box.missIndex;
                continue;
            }
            else
            return false;
        }
    }
    return false;
}
vertexout VS(vertexin pin)
{
    vertexout vout = (vertexout) 0;
    float4 Position = mul(pin.position,world);
    float4 posW = Position;
    vout.positionH = mul(Position, VP);
    vout.positionW = posW;
    vout.uv = pin.uv;
    vout.color = pin.color;
    vout.normal = pin.noraml;
    return vout;
}
float4 PS(vertexout pin) :SV_Target
{
    float roughness = 0.4f;
    float3 F0 = float3(0.5, 0.5, 0.5);
    float4 texcolor = g_tex[texIndex].Sample(g_sampler, pin.uv);
    float3 toEye = normalize(eyepos - pin.positionW);
    float3 norL = normalize(lightdir);
    float3 norN = normalize(pin.normal);
    float3 diffuse = smooth_SurfaceDiff(F0, texcolor.xyz, norN, norL, dot(norN, toEye));
    float3 h = normalize(toEye + norL);
    float3 F = SchlickFresnel(F0, norN, h);
    float G = smithG2(h, norL, norN, roughness, dot(norN, toEye), dot(h, toEye));
    float D = GGX(roughness, norN, h);
    float3 BRDF = G * D * F / (4 * abs(dot(norN, norL)) * abs(dot(norN, toEye))) + diffuse;
    float3 r = normalize(-1.0f * toEye - (2 * dot(-1.0f * toEye, pin.normal)) * pin.normal);
    ray R;
    R.direction = r;
    R.origin = pin.positionW;
    MyTriangle t;
    float2 b1b2;
    if (RayIntersectScene(R, t,b1b2))
    {
        float u = t.uv12.x * (1.0f - b1b2.x - b1b2.y) + t.uv12.z * b1b2.x + t.uv3.x * b1b2.y;
        float v = t.uv12.y * (1.0f - b1b2.x - b1b2.y) + t.uv12.w * b1b2.x + t.uv3.y * b1b2.y;
        return g_tex[t.texIndex].Sample(g_sampler, float2(u,v));
    }
    return float4(BRDF, 1.0f);
}