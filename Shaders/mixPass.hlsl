
Texture2D<float4> SrcTexture : register(t0);
Texture2D<float4> g_worldpos : register(t1);
Texture2D<float4> g_normal : register(t2);
Texture2D<float4> g_lastnormal : register(t7);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState g_sampler : register(s0);
#define width 1024
#define height 768
cbuffer lastVP : register(b0)
{
    float4x4 lastVP;
    float4x4 last6VP;
    uint nframe;
}
[numthreads(16, 16, 1)]
void CS(uint2 GroupId : SV_GroupID,
				 uint GroupThreadIndex : SV_GroupIndex,
				 uint2 DispatchThreadId : SV_DispatchThreadID)
{
    float3 wpos = g_worldpos[DispatchThreadId*2].xyz * 400.0f - 200.0f;
    float4 backProjectCoord = mul(float4(wpos, 1.0f), lastVP);
    float2 backProjectScreenCoord = ((backProjectCoord / backProjectCoord.w).xy + 1.0f) / 2.0f;
    backProjectScreenCoord.y = (1.0f - backProjectScreenCoord.y); 
    int2 lastIndex = int2(backProjectScreenCoord * int2(width, height)); //通过motionVector得到上一帧坐标
    float4 lastFrameColor = SrcTexture[lastIndex];
    float mixRate;
    if (backProjectScreenCoord.x > 0 && backProjectScreenCoord.x < 1.0f && backProjectScreenCoord.y > 0 && backProjectScreenCoord.y < 1.0f)//可以通过motionVector找到上一帧此像素对应的moment
    {
        float3 lastNormal = normalize(g_lastnormal[lastIndex * 2].xyz * 2.0f - 1.0f);
        float3 currentNormal = normalize(g_normal[DispatchThreadId * 2].xyz * 2.0f - 1.0f);
        if (abs(dot(lastNormal, currentNormal)) < 0.9f && nframe > 0)//此像素是disocclusion新出现的像素,不复用上一帧
            mixRate = 0;
        else
            mixRate = 0.8f;
    }
    else //此像素是通过变换视角新出现的像素
        mixRate = 0;
    if (nframe == 0)
        mixRate = 0;
    float4 curColor = DstTexture[DispatchThreadId];
    DstTexture[DispatchThreadId] = (1.0f - mixRate) * curColor + mixRate * lastFrameColor;
    GroupMemoryBarrierWithGroupSync();
}