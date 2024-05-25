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
    float4 texcolor = (texIndex >= 0) ? g_tex[texIndex].Sample(g_sampler, pin.uv) : float4(pin.color, 1.0f);
    float2 SSuv = float2((pin.positionH.xy) / float2(width, height));
    float4 irradianceDivPI = g_DirectL.Sample(g_sampler, SSuv) + g_IndirectL.Sample(g_sampler, SSuv);
    return irradianceDivPI * texcolor;
}