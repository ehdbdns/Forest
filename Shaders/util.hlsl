#define PI 3.1415926f
#define width 1024
#define height 768
struct ray
{
    float3 direction;
    float3 origin;
};
struct AABBbox
{
    float3 center;
    uint isLeaf;
    float3 extent;
    int missIndex;
    uint triangleStart;
    uint triangleNum;
};
struct MyTriangle
{
    float3 pos1;
    int materialIndex;
    float3 pos2;
    int texIndex;
    float3 pos3;
    float pad1;
    float3 color;
    float pad2;
    float3 n;
    float pad3;
    float4 uv12;
    float2 uv3;
};
struct material
{
    float3 albedo;
    float roughness;
    float3 F0;
};
struct PolygonalLight
{
    float area;
    float Xstart;
    float Xend;
    float Zstart;
    float Zend;
    float3 color;
    float3 normal;
};
struct vertexin
{
    float4 position : POSITION;
    float2 uv : TEXCOORD;
    float3 noraml : NORMAL;
    float3 tangentU : TANGENT;
    float AOk : AOk;
    float3 color : COLOR;
};
struct vertexout
{
    float4 positionH : SV_POSITION;
    float3 positionW : POSITIONT;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tanegntU : TANGENT;
    float3 color : COLOR;
};

Texture2D g_tex[8] : register(t0);
StructuredBuffer<AABBbox> g_boxData : register(t0, space1);
StructuredBuffer<MyTriangle> g_triangleData : register(t1, space1);
StructuredBuffer<material> g_materialData : register(t2, space1);
StructuredBuffer<float> g_randnums : register(t3, space1);
StructuredBuffer<PolygonalLight> g_lights : register(t4, space1);
Texture2D g_lastFrame : register(t5, space1);
Texture2D g_DirectL : register(t6, space1);
Texture2D g_IndirectL : register(t7, space1);
SamplerState g_sampler : register(s0);
cbuffer passcb : register(b1)
{
    float4x4 MWVP;
    float3 eyepos;
    float pad1;
    float3 AL;
    float pad2;
    float3 BL;
    float m;
    float3 lightdir;
    float pad3;
    float4 md;
    float3 r0;
    float pad4;
    float4x4 V;
    float4x4 W;
    float4x4 VP;
    float4x4 WinvT;
    float4x4 P;
    float4x4 S;
    uint boxNum;
    uint nFrame;
    uint curtime;
}
cbuffer objcb : register(b0)
{
    float4x4 world;
    float4x4 worldinvT;
    int texIndex;
    int matIndex;
}
int Xplus(float alpha)
{
    if (alpha > 0)
        return 1;
    else
        return 0;
}
float GGX(float3 alpha, float3 n, float3 h)
{
    alpha = alpha * alpha;
    float ret = (pow(alpha, 2) * Xplus(dot(h, n))) / (3.1415926585f * pow((1 + pow(dot(n, h), 2) * (alpha * alpha - 1)), 2));
    if (ret > 10)
        return 10;
    
    return ret;
}
float3 SchlickFresnel(float3 F0, float3 n, float3 h)
{
    return F0 + (1 - F0) * pow((1 - max(0, dot(n, h))), 5);
    

}
float3 smooth_SurfaceDiff(float3 F0, float3 albedo, float3 n, float3 l, float Dotnv)
{
    float k = (21 / (20 * 3.1415926));
    float3 base1 = (1 - pow(1 - max(0, dot(n, l)), 5));
    float3 base2 = (1 - pow(1 - max(0, Dotnv), 5));
    return k * (1 - F0) * base1 * base2 * albedo;

}
float lambda(float alpha)
{
    return (-1 + pow((1 + (1 / dot(alpha, alpha))), 0.5)) / 2;

}
float smithG2(float3 h, float3 l, float3 n, float roughness, float Dotnv, float Dothv)
{
    float a1 = dot(n, l) / (roughness * (pow((1 - pow(dot(n, l), 2)), 0.5)));
    float a2 = Dotnv / (roughness * (pow((1 - pow(Dotnv, 2)), 0.5)));
    float3 base1 = 1 + lambda(a1) + lambda(a2);
    float3 base2 = Xplus(Dothv) * Xplus(dot(h, l));
    return base2 / base1;
}
float4 UniformSampleHemisphere(float2 E,float3 n)
{
    float Phi = 2 * 3.1415926f * E.x;
    float CosTheta = E.y;
    float SinTheta = sqrt(1 - CosTheta * CosTheta);

    float3 H;
    H.x = SinTheta * cos(Phi);
    H.y = SinTheta * sin(Phi);
    H.z = CosTheta;
    float PDF = 1.0 / (2 * PI);
    float3 up = abs(n.z) < 0.999 ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
    float3 tangent = normalize(cross(up, n));
    float3 bitangent = cross(n, tangent);
    float3 sampleVec = tangent * H.x + bitangent * H.y + n * H.z; //注意是左乘矩阵
    return float4(normalize(sampleVec), PDF);
}
bool RayIntersectTriangle(float3 pos0, float3 pos1, float3 pos2, ray r, out float2 b1b2,out float tout)
{
    float3 E1 = pos1 - pos0;
    float3 E2 = pos2 - pos0;
    float3 S = r.origin - pos0;
    float3 S1 = cross(r.direction, E2);
    float3 S2 = cross(S, E1);
    float c = 1 / dot(S1, E1);
    float t = c * dot(S2, E2);
    float b1 = c * dot(S1, S);
    float b2 = c * dot(S2, r.direction);
    if (b1 > 0 && b2 > 0 && (1 - b1 - b2) > 0 && t > 0)
    {
        tout = t;
        b1b2 = float2(b1, b2);
        return true;
    }
    return false;
}
bool RayIntersectAABBBox(float3 boxCenter, float3 boxExtent, ray r)
{
    float tminx, tminy, tminz;
    float tmaxx, tmaxy, tmaxz;
    float t1, t2;
    t1 = (boxCenter.x - boxExtent.x - r.origin.x) / r.direction.x;
    t2 = (boxCenter.x + boxExtent.x - r.origin.x) / r.direction.x;
    tminx = (t2 > t1) ? t1 : t2;
    tmaxx = (t2 > t1) ? t2 : t1;
    t1 = (boxCenter.y - boxExtent.y - r.origin.y) / r.direction.y;
    t2 = (boxCenter.y + boxExtent.y - r.origin.y) / r.direction.y;
    tminy = (t2 > t1) ? t1 : t2;
    tmaxy = (t2 > t1) ? t2 : t1;
    t1 = (boxCenter.z - boxExtent.z - r.origin.z) / r.direction.z;
    t2 = (boxCenter.z + boxExtent.z - r.origin.z) / r.direction.z;
    tminz = (t2 > t1) ? t1 : t2;
    tmaxz = (t2 > t1) ? t2 : t1;
    float tEnter = max(max(tminx, tminy), tminz);
    float tExit = min(min(tmaxx, tmaxy), tmaxz);
    if (tEnter < tExit && tExit > 0)
        return true;
    return false;
}
bool RayIntersectScene(ray r, out MyTriangle outtri, out float2 b1b2,out float tout)
{
    AABBbox box;
    int i = 0;
    while (i < boxNum)
    {
        box = g_boxData[i];
        if (RayIntersectAABBBox(box.center, box.extent, r))
        {
            if (box.isLeaf)
            {
                for (int j = 0; j < box.triangleNum; j++)
                {
                    MyTriangle tri = g_triangleData[box.triangleStart + j];
                    float t;
                    if (RayIntersectTriangle(tri.pos1, tri.pos2, tri.pos3, r, b1b2,t))
                    {
                        outtri = tri;
                        tout = t;
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
float3 calcDirectLightFromPolygonalLight(float3 shadingPoint, float3 spNormal,float3 BRDF,float2 uv)
{
    float randnum1 = g_randnums[(100000 * uv.x) % 1000];
    float randnum2 = g_randnums[(100000 * uv.y) % 1000];
    float dx = g_lights[0].Xend - g_lights[0].Xstart;
    float dz = g_lights[0].Zend - g_lights[0].Zstart;
    float3 sampleLightPos = float3(g_lights[0].Xstart + randnum1 * dx, 199.5f, g_lights[0].Zstart + randnum2 * dz);
    float3 posToLight = sampleLightPos - shadingPoint;
    //ray r;
    //r.origin = shadingPoint;
    //r.direction = normalize(posToLight);
    //MyTriangle tri;
    //float2 b1b2;
    //float t=-1.0f;
    //if (RayIntersectScene(r, tri, b1b2,t))
    //{
    //    if (t < posToLight.x / r.direction.x)
    //    return float3(0, 0, 0);
    //}
    float3 toLight =sampleLightPos - shadingPoint;
    float3 ToLightNorm = normalize(toLight);
    float cos1 = max(0, dot(spNormal, ToLightNorm));
    float cos2 = max(0, dot(normalize(g_lights[0].normal), -ToLightNorm));
    float LengthSquare = toLight.x * toLight.x + toLight.y * toLight.y + toLight.z * toLight.z;
    return BRDF * g_lights[0].color * cos1 * cos2 / LengthSquare * g_lights[0].area*100;
}
float3 calcDirectLightInFirstFrame(float3 shadingPoint, float3 spNormal, float3 BRDF,float2 uv)
{
    float3 DirectL = float3(0, 0, 0);
    for (int i = 0; i < 100; i++)
    {
        float randnum1 = g_randnums[(100000 * uv.x) % 1000+i];
        float randnum2 = g_randnums[(100000 * uv.y) % 1000+i];
        float dx = g_lights[0].Xend - g_lights[0].Xstart;
        float dz = g_lights[0].Zend - g_lights[0].Zstart;
        float3 sampleLightPos = float3(g_lights[0].Xstart + randnum1 * dx, 199.5f, g_lights[0].Zstart + randnum2 * dz);
        float3 toLight = sampleLightPos - shadingPoint;
        float3 ToLightNorm = normalize(toLight);
        float cos1 = max(0, dot(spNormal, ToLightNorm));
        float cos2 = max(0, dot(normalize(g_lights[0].normal), -ToLightNorm));
        float LengthSquare = toLight.x * toLight.x + toLight.y * toLight.y + toLight.z * toLight.z;
        DirectL+= BRDF * g_lights[0].color * cos1 * cos2 / LengthSquare * g_lights[0].area * 100;
    }
    return DirectL / 100.0f;
}