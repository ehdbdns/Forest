Texture2D<float4> SrcTexture : register(t0);
Texture2D<float4> g_worldpos : register(t1);
Texture2D<float4> g_normal : register(t2);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState g_sampler : register(s0);
float3 clamp(float3 c1, float3 c2, float3 input, out bool isOutlier)
{
    float minx, miny, minz;
    float maxx, maxy, maxz;
    float3 output;
    bool b = false;
    minx = (c1.x > c2.x) ? c2.x : c1.x;
    maxx = (c1.x < c2.x) ? c2.x : c1.x;
    miny = (c1.y > c2.y) ? c2.y : c1.y;
    maxy = (c1.y < c2.y) ? c2.y : c1.y;
    minz = (c1.z > c2.z) ? c2.z : c1.z;
    maxz = (c1.z < c2.z) ? c2.z : c1.z;
    if (input.x >= minx && input.x <= maxx)
        output.x = input.x;
    else
    {
        output.x = minx;
        b = true;
    }
    if (input.y >= miny && input.y <= maxy)
        output.y = input.y;
    else
    {
        output.y = miny;
        b = true;
    }
    if (input.z >= minz && input.z <= maxz)
        output.z = input.z;
    else
    {
        output.z = minz;
        b = true;
    }
    isOutlier = (b) ? true : false;
    return output;
}
[numthreads(16, 16, 1)]
void CS(uint2 GroupId : SV_GroupID,
				 uint GroupThreadIndex : SV_GroupIndex,
				 uint2 DispatchThreadId : SV_DispatchThreadID)
{
    float3 sumX = float3(0, 0, 0);
    float3 sumX2 = float3(0, 0, 0);
    for (int i = -3; i < 4; i++)
    {
        for (int j = -3; j < 4; j++)
        {
            float3 c = SrcTexture[DispatchThreadId + int2(i, j)].xyz;
            sumX += c;
            sumX2 += c * c;
        }
    }
    float3 EX = sumX / 49;
    float3 EX2 = sumX2 / 49;
    float3 D = EX2 - EX * EX; //得到当前像素周围统计学数据
    bool isOutlier = false;
    float4 clampedColor = float4(clamp(EX - 80 * D, EX + 80 * D, SrcTexture[DispatchThreadId].xyz, isOutlier), 1.0f);
   if(isOutlier)
        DstTexture[DispatchThreadId] = clampedColor;
    else
        DstTexture[DispatchThreadId] = SrcTexture[DispatchThreadId];
    GroupMemoryBarrierWithGroupSync();
}