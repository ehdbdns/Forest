Texture2D<float4> SrcTexture : register(t0);
Texture2D<float4> g_worldpos : register(t1);
Texture2D<float4> g_normal : register(t2);
StructuredBuffer<int2> g_offsets : register(t3);
StructuredBuffer<float> g_h : register(t4);
Texture2D<float4> g_z : register(t5);
Texture2D<float4> g_gz : register(t6);
RWTexture2D<float4> DstTexture : register(u0);
RWTexture2D<float4> moment1tex : register(u1);
RWTexture2D<float4> moment2tex : register(u2);
RWTexture2D<float4> historytex : register(u3);
RWTexture2D<float4> variancetex : register(u4);
SamplerState g_sampler : register(s0);
float squareLength(float3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
float squareLength(int2 v)
{
    return float(v.x * v.x + v.y * v.y);
}
float3 calcJBFWeight(int2 i,int2 j,float3 variance)
{
    float InvSigmaRT = 1.0f/500.0f;
    float SigmaN = 128.0f;
    float InvSigmaZ = 1.0f/60.0f;
    float3 eRT = length(SrcTexture[i].xyz - SrcTexture[j].xyz) * InvSigmaRT/ (sqrt(variance) + float3(0.001,0.001,0.001));
    float wN = pow(max(0, dot(g_normal[i*2].xyz*2.0f-1.0f, g_normal[j*2].xyz*2.0f-1.0f)), SigmaN);
    float eZ = length(g_z[i*2].x - g_z[j*2].x) * InvSigmaZ / (length(g_gz[i*2].x * (i - j)) + float3(0.001,0.001,0.001));
    return wN* exp(-eRT);
}
[numthreads(16, 16, 1)]
void CS(uint2 GroupId : SV_GroupID,
				 uint GroupThreadIndex : SV_GroupIndex,
				 uint2 DispatchThreadId : SV_DispatchThreadID)
{
    float3 totalColorWeight = float3(0,0,0);
    float3 totalVarianceWeight = float3(0,0,0);
    float3 totalColor = float3(0, 0, 0);
    float3 totalVariance = float3(0, 0, 0);
    float3 currentVariance = variancetex[DispatchThreadId].xyz;
    for (int i = 0; i < 25; i++)
    {
        int2 j = DispatchThreadId + g_offsets[i];
        float3 w = calcJBFWeight(DispatchThreadId, j,currentVariance);
        float h = g_h[i % 5];
        float3 weight = h*w;
        totalColor += weight * SrcTexture[j].xyz;
        totalVariance += weight * weight * variancetex[j].xyz;
        totalColorWeight += weight;
        totalVarianceWeight += weight * weight;

    }
    totalColor /= totalColorWeight;
    totalVariance /= totalVarianceWeight;
    DstTexture[DispatchThreadId] = float4(totalColor, 1.0f);
    variancetex[DispatchThreadId] = float4(totalVariance, 1.0f);
    GroupMemoryBarrierWithGroupSync();
}