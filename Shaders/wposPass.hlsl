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
    return float4((pin.positionW + 200.0f) / 400.0f, 1.0f);

}