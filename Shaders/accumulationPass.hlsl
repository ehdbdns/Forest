Texture2D<float4> currentFrame : register(t0);
Texture2D<float4> g_worldpos : register(t1);
Texture2D<float4> g_normal : register(t2);
StructuredBuffer<int2> g_offsets : register(t3);
StructuredBuffer<float> g_h : register(t4);
Texture2D<float4> g_lastnormal : register(t7);
RWTexture2D<float4> DstTexture : register(u0);
RWTexture2D<float4> moment1tex : register(u1);
RWTexture2D<float4> moment2tex : register(u2);
RWTexture2D<float4> historytex : register(u3);
RWTexture2D<float4> variancetex : register(u4);
RWTexture2D<float4> lastMoment1tex : register(u5);
RWTexture2D<float4> lastMoment2tex : register(u6);
#define width 1024
#define height 768
cbuffer lastVP : register(b0)
{
    float4x4 VP;
    float4x4 sumVP;
    uint nframe;
}
SamplerState g_sampler : register(s0);

[numthreads(16, 16, 1)]
void CS(uint2 GroupId : SV_GroupID,
				 uint GroupThreadIndex : SV_GroupIndex,
				 uint2 DispatchThreadId : SV_DispatchThreadID)
{
    float3 wpos = g_worldpos[DispatchThreadId * 2].xyz * 400.0f - 200.0f;
    float4 backProjectCoord = mul(float4(wpos, 1.0f), VP);
    float2 backProjectScreenCoord = ((backProjectCoord / backProjectCoord.w).xy + 1.0f) / 2.0f;
    backProjectScreenCoord.y = (1.0f - backProjectScreenCoord.y); 
    int2 lastIndex = int2(backProjectScreenCoord * int2(width, height)); //通过motionVector得到上一帧坐标
    
    float4 curColor = currentFrame[DispatchThreadId];
    if(backProjectScreenCoord.x>0&&backProjectScreenCoord.x<1.0f&&backProjectScreenCoord.y>0&&backProjectScreenCoord.y<1.0f)//可以通过motionVector找到上一帧此像素对应的moment
    {
        float3 lastNormal = normalize(g_lastnormal[lastIndex * 2].xyz * 2.0f - 1.0f);
        float3 currentNormal = normalize(g_normal[DispatchThreadId * 2].xyz * 2.0f - 1.0f);
        if (abs(dot(lastNormal, currentNormal)) < 0.9f && nframe > 1)//此像素是disocclusion新出现的像素
        {
            moment1tex[DispatchThreadId] = float4(((1.0f / 6.0f) * curColor).xyz, 0.1f);
            moment2tex[DispatchThreadId] = float4(((1.0f / 6.0f) * curColor * curColor).xyz, 0.1f);
            return;
        }
        float4 historyColor = float4(0, 0, 0, 0);
        float4 lastMoment1 = lastMoment1tex[lastIndex];
        float4 lastMoment2 = lastMoment2tex[lastIndex];
        if(lastMoment1.w>0.55f)//此像素的历史帧中的有效帧等于6
        {
            float4 back6ProjectCoord = mul(float4(wpos, 1.0f), sumVP);
            float2 back6ProjectScreenCoord = ((back6ProjectCoord / back6ProjectCoord.w).xy + 1.0f) / 2.0f;
            back6ProjectScreenCoord.y = (1.0f - back6ProjectScreenCoord.y);
            int2 last6Index = int2(back6ProjectScreenCoord * int2(width, height));//通过六帧累计motionVector获得六帧前的坐标,这个坐标一定是有效的
            historyColor = historytex[last6Index];
            lastMoment1.xyz -= float3((1.0f / 6.0f) * historyColor.xyz);
            lastMoment1.xyz += float3((1.0f / 6.0f) * curColor.xyz);
            moment1tex[DispatchThreadId] = lastMoment1;
            lastMoment2.xyz -= float3(((1.0f / 6.0f) * historyColor * historyColor).xyz);
            lastMoment2.xyz += float3(((1.0f / 6.0f) * curColor * curColor).xyz);
            moment2tex[DispatchThreadId] = lastMoment2;
        }
        else
        {
            lastMoment1 += float4(((1.0f / 6.0f) * curColor).xyz, 0.1f);
            moment1tex[DispatchThreadId] = lastMoment1;
            lastMoment2 += float4(((1.0f / 6.0f) * curColor * curColor).xyz, 0.1f);
            moment2tex[DispatchThreadId] = lastMoment2;
        }
    }
    else//此像素是通过变换视角新出现的像素
    {
        moment1tex[DispatchThreadId] = float4(((1.0f / 6.0f) * curColor).xyz, 0.1f);
        moment2tex[DispatchThreadId] = float4(((1.0f / 6.0f) * curColor*curColor).xyz, 0.1f);
    }
    GroupMemoryBarrierWithGroupSync();
}