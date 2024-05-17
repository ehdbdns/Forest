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
    vout.normal = mul(float4(pin.noraml,1.0f),worldinvT);
    return vout;
}
float4 PS(vertexout pin) : SV_Target
{
    float roughness = 0.4f;
    float3 F0 = float3(1.0, 1.0, 1.0);
    float3 texcolor = g_tex[texIndex].Sample(g_sampler, pin.uv).xyz;
    float3 toEye = normalize(eyepos - pin.positionW);
    float3 norL = normalize(lightdir);
    float3 norN = normalize(pin.normal);
    return float4(texcolor/PI, 1.0f);
}