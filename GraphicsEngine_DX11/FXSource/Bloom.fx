// Bloom.fx
// 블룸 커브 적용
// 일정 밝기 이상을 추출 + 일정 밝기 이하는 더 어둡게

float gThreshold;

#define BLOOMCURVE_METHOD_1
//#define BLOOMCURVE_METHOD_2
//#define BLOOMCURVE_METHOD_3

float GetBloomCurve(float x)
{
    float result = x;
    x *= 2.0f;

#ifdef BLOOMCURVE_METHOD_1
    result = x * 0.05 + max(0, x - gThreshold) * 0.5; // default gThreshold = 1.26
#endif

#ifdef BLOOMCURVE_METHOD_2
    result = x * x / 3.2;
#endif

#ifdef BLOOMCURVE_METHOD_3
    result = max(0, x - gThreshold); // default gThreshold = 1.0
    result *= result;
#endif 

    return result * 0.5f;
}

float4x4  gWorldViewProj;
Texture2D gTexture;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
};
 
VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex  = vin.Tex;
	
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float3 color = gTexture.Sample(samLinear, pin.Tex).xyz;

	float intensity = dot(color, float3(0.3f, 0.3f, 0.3f));
	float bloomIntensity = GetBloomCurve(intensity);
	float3 bloomColor = color * bloomIntensity / intensity;
	
	return float4(bloomColor.xyz, 1.0);
}

technique11 BloomCurve
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}
