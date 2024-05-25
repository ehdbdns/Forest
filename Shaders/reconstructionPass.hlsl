Texture2D<float4> SrcTexture : register(t0);
Texture2D<float4> g_worldpos : register(t1);
Texture2D<float4> g_normal : register(t2);
StructuredBuffer<int2> g_offsets : register(t3);
StructuredBuffer<float> g_h : register(t4);
RWTexture2D<float4> DstTexture : register(u0);
RWTexture2D<float4> FinalTex : register(u1);
SamplerState g_sampler : register(s0);
[numthreads(16, 16, 1)]
void CS(uint2 GroupId : SV_GroupID,
				 uint GroupThreadIndex : SV_GroupIndex,
				 uint2 DispatchThreadId : SV_DispatchThreadID)
{
    DstTexture[DispatchThreadId] += SrcTexture[DispatchThreadId];
    GroupMemoryBarrierWithGroupSync();
}