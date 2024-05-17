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
float4 PS(vertexout pin) :SV_Target
{
    float4 texcolor = (texIndex >= 0) ? g_tex[texIndex].Sample(g_sampler, pin.uv) : float4(pin.color,1.0f);
    float3 toEye = normalize(eyepos - pin.positionW);
    float3 norL = normalize(lightdir);
    float3 norN = normalize(pin.normal);
    //float roughness = 0.4f;
    //float3 F0 = float3(0.5, 0.5, 0.5);
    //float3 diffuse = smooth_SurfaceDiff(F0, texcolor.xyz, norN, norL, dot(norN, toEye));
    //float3 h = normalize(toEye + norL);
    //float3 F = SchlickFresnel(F0, norN, h);
    //float G = smithG2(h, norL, norN, roughness, dot(norN, toEye), dot(h, toEye));
    //float D = GGX(roughness, norN, h);
    //float3 BRDF = G * D * F / (4 * abs(dot(norN, norL)) * abs(dot(norN, toEye))) + diffuse;
    //float3 r = normalize(-1.0f * toEye - (2 * dot(-1.0f * toEye, pin.normal)) * pin.normal);
    
    float3 BRDF = texcolor.xyz / PI;
    float3 DirectL = calcDirectLightFromPolygonalLight(pin.positionW, norN, BRDF, pin.uv);
    float4 traceRayAndPdf = float4(0, 0, 0, 0);
    ray R;
    R.origin = pin.positionW;
    float3 IndirectL = float3(0,0,0);
    MyTriangle tri;
    float2 b1b2;
    float3 accumulateBRDF = BRDF;
    for (int i = 0; i < 2; i++)
    {
        float2 random = float2(g_randnums[pin.uv.x * 10000%1000], g_randnums[pin.uv.y * 10000%1000]);
        traceRayAndPdf = UniformSampleHemisphere(random, norN);
        R.direction = traceRayAndPdf.xyz;
        float t;
        if (RayIntersectScene(R, tri, b1b2,t))
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
    return float4(IndirectL+DirectL, 1.0f);
}