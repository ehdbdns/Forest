Texture2D<float4> SrcTexture : register(t0);
Texture2D<float4> g_worldpos : register(t1);
Texture2D<float4> g_normal : register(t2);
RWTexture2D<float4> DstTexture : register(u0);
SamplerState g_sampler : register(s0);
float squareLength(float3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}
float squareLength(int2 v)
{
    return float(v.x * v.x + v.y * v.y);
}
float calcJBFWeight(int2 i,int2 j)
{
    float InvSigmaP = 1.0f;
    float InvSigmaC = 1.0f;
    float InvSigmaN = 1.0f;
    float InvSigmaD = 1.0f;
    float DP = squareLength(i - j)*InvSigmaP*InvSigmaP*0.5f;
    float DC = squareLength(SrcTexture[i].xyz-SrcTexture[j].xyz)*InvSigmaC*InvSigmaC*0.5f;
    float dotNN = dot(normalize(g_normal[i].xyz * 2.0f - 1.0f), normalize(g_normal[j].xyz * 2.0f - 1.0f));
    float Dnormal = (dotNN>0.99f)?0:acos(dotNN);
    float DN = Dnormal * Dnormal * InvSigmaN * InvSigmaN * 0.5f;
    float Dplane = dot(normalize(g_normal[i].xyz*2.0f-1.0f), normalize(g_worldpos[j].xyz*200.0f-100.0f - g_worldpos[i].xyz*200.0f-100.0f));
    float DD = Dplane*Dplane*InvSigmaD*InvSigmaD*0.5f;
    return exp(-DP - DC - DN - DD);
}
[numthreads(16, 16, 1)]
void CS(uint2 GroupId : SV_GroupID,
				 uint GroupThreadIndex : SV_GroupIndex,
				 uint2 DispatchThreadId : SV_DispatchThreadID)
{
    float totalWeight = 0;
    float3 totalColor = float3(0, 0, 0);
    for (int x = -2; x < 3; x++)
    {
        for (int y = -2; y < 3; y++)
        {
            int2 j = DispatchThreadId + int2(x, y);
            float weight = calcJBFWeight(DispatchThreadId, j);
            totalColor += weight * SrcTexture[j].xyz;
            totalWeight += weight;
        }
    }
    totalColor /= totalWeight;
        DstTexture[DispatchThreadId] = float4(totalColor,1.0f);
   
    GroupMemoryBarrierWithGroupSync();
}