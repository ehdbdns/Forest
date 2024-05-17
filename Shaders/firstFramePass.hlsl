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
    vout.normal = mul(float4(pin.noraml, 1.0f), worldinvT);
    return vout;
}
float4 PS(vertexout pin) : SV_Target
{
    float3 texcolor = (pin.color.x<0)?g_tex[texIndex].Sample(g_sampler, pin.uv):pin.color;
    //float3 toEye = normalize(eyepos - pin.positionW);
    float3 norN = normalize(pin.normal);
    float3 BRDF = texcolor / PI;
    float3 DirectL = calcDirectLightInFirstFrame(pin.positionW, norN, BRDF, pin.uv);
    return float4(DirectL, 1.0f);
}