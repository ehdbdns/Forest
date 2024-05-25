Texture2D<float4> SrcTexture : register(t0);
Texture2D<float4> g_worldpos : register(t1);
Texture2D<float4> g_normal : register(t2);
StructuredBuffer<int2> g_offsets : register(t3);
StructuredBuffer<float> g_h : register(t4);
RWTexture2D<float4> DstTexture : register(u0);
RWTexture2D<float4> moment1tex : register(u1);
RWTexture2D<float4> moment2tex : register(u2);
RWTexture2D<float4> historytex : register(u3);
RWTexture2D<float4> variancetex : register(u4);
SamplerState g_sampler : register(s0);
float3 calcVarianceWeight(int2 i, int2 j)
{
    float SigmaN = 128.0f;
    float wN = pow(max(0, dot(g_normal[i * 2].xyz * 2.0f - 1.0f, g_normal[j * 2].xyz * 2.0f - 1.0f)), SigmaN);
    return wN;
}
[numthreads(16, 16, 1)]
void CS(uint2 GroupId : SV_GroupID,
				 uint GroupThreadIndex : SV_GroupIndex,
				 uint2 DispatchThreadId : SV_DispatchThreadID)
{
    float4 moment1 = moment1tex[DispatchThreadId];
    if(moment1.w<0.32f)
    {
        float4 sumWeight = float4(0, 0, 0, 0);
        float4 sumVariance = float4(0, 0, 0, 0);
        for (int i = 0; i < 25; i++)
        {
            float w = calcVarianceWeight(DispatchThreadId, g_offsets[i])*g_h[i%5];
            sumWeight += w;
            sumVariance += (w * variancetex[g_offsets[i]]);
        }
        sumVariance /= sumWeight;
        variancetex[DispatchThreadId] = sumVariance;
    }
    else
    variancetex[DispatchThreadId] = sqrt(moment2tex[DispatchThreadId] - moment1 * moment1);
    GroupMemoryBarrierWithGroupSync();
}