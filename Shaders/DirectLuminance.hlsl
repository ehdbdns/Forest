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
    float3 BRDF = 1.0f / PI;
    float3 DirectL = calcDirectLightFromPolygonalLight(pin.positionW, norN, BRDF, pin.uv);
    return float4(DirectL, 1.0f);
}