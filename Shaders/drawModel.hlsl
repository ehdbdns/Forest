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
    float roughness = 0.4f;
    float3 F0 = float3(1.0, 1.0, 1.0);
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
    return texcolor;
}