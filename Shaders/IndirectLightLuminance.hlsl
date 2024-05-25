#include"util.hlsl"

vertexout VS(vertexin pin)
{
    vertexout vout = (vertexout) 0;
    float4 Position = mul(pin.position, world);
    float4 posW = Position;
    vout.positionH = mul(Position, VP);
    vout.positionW = posW;
    vout.uv = pin.uv;
    vout.color = pin.color;
    vout.normal = pin.noraml;
    return vout;
}
float4 PS(vertexout pin) : SV_Target
{
    float4 texcolor = (texIndex >= 0) ? g_tex[texIndex].Sample(g_sampler, pin.uv) : float4(pin.color, 1.0f);
    float3 toEye = normalize(eyepos - pin.positionW);
    float3 norL = normalize(lightdir);
    float3 norN = normalize(pin.normal);
    float3 BRDF = 1.0f / PI;
    float4 traceRayAndPdf = float4(0, 0, 0, 0);
    ray R;
    R.origin = pin.positionW;
    float3 IndirectL = float3(0, 0, 0);
    MyTriangle tri;
    float2 b1b2;
    float3 accumulateBRDF = BRDF;
    for (int i = 0; i < 2; i++)
    {
        float2 random = float2(g_randnums[pin.uv.x * 10000 % 1000], g_randnums[pin.uv.y * 10000 % 1000]);
        traceRayAndPdf = UniformSampleHemisphere(random, norN);
        R.direction = traceRayAndPdf.xyz;
        float t;
        if (RayIntersectScene(R, tri, b1b2, t))
        {
            float cosTheta = dot(R.direction, norN);
            float3 hitPoint = tri.pos1 * (1 - b1b2.x - b1b2.y) + tri.pos2 * b1b2.x + tri.pos3 * b1b2.y;
            float u = tri.uv12.x * (1.0f - b1b2.x - b1b2.y) + tri.uv12.z * b1b2.x + tri.uv3.x * b1b2.y;
            float v = tri.uv12.y * (1.0f - b1b2.x - b1b2.y) + tri.uv12.w * b1b2.x + tri.uv3.y * b1b2.y;
            float3 hptexcolor = (tri.texIndex == -1) ? tri.color : g_tex[tri.texIndex].Sample(g_sampler, float2(u, v)).xyz;
            float3 hpBRDF = hptexcolor / PI;
            IndirectL += accumulateBRDF * cosTheta / traceRayAndPdf.w * calcDirectLightFromPolygonalLight(hitPoint, normalize(tri.n), hpBRDF, tri.uv3);
            accumulateBRDF *= hpBRDF;
        }
    }
    return float4(IndirectL, 1.0f);
}