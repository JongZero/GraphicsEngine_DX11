// Tone Mapping
// ACES Filmic Tone Mapping
// Reinhard Tone Mapping
// Simple Tone Mapping -> AA 하기 전, 후에 사용

cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
};

SamplerState samLinear
{
	//Filter = ANISOTROPIC;
	//MaxAnisotropy = 4;
	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = WRAP;
	AddressV = WRAP;
};

Texture2D gInput;

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

float max3(float x, float y, float z) { return max(x, max(y, z)); }
float rcpf(float x) { return 1.0 / x; }

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	//vout.PosH = float4(vin.PosL.xy, 1.0, 1.0);
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS_ToneMapping_Simple(VertexOut pin) : SV_Target
{
	float4 color = gInput.Sample(samLinear, pin.Tex);
	//color =  color * rcpf(max3(color.r, color.g, color.b) + 1.0);
	color = pow(color, 1 / 2.2f);
	return float4(color.xyz, 1.0f);
}

float4 PS_ToneMapping_Simple_Invert(VertexOut pin) : SV_Target
{
	float4 color = gInput.Sample(samLinear, pin.Tex);
	color = color * rcpf(1.0 - max3(color.r, color.g, color.b));
	return float4(color.xyz, 1.0f);
}

float4 PS_ToneMapping_ACES_Filmic(VertexOut pin) : SV_Target
{
	float3 color = gInput.Sample(samLinear, pin.Tex).xyz;

	// exposure
	//float exposure = 1.0f;
	//color = float3(1.0f, 1.0f, 1.0f) - exp(-color * exposure);

	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;

	color = saturate((color * (a * color + b)) / (color * (c * color + d) + e));

	// 라이트 연산 및 톤 매핑이 끝났으므로 다시 Encoding 공간으로 보낸다.
	color = pow(color, 1 / 2.2f);

	return float4(color.xyz, 1.0f);
}

float4 PS_ToneMapping_Reinhard(VertexOut pin) : SV_Target
{
	float4 color = gInput.Sample(samLinear, pin.Tex);

	float k = 1.0f;
	color.xyz = color.xyz / (color.xyz + k);

	// 라이트 연산 및 톤 매핑이 끝났으므로 다시 Encoding 공간으로 보낸다.
	color = pow(color, 1 / 2.2f);

	return color;
}

technique11 ToneMapping_Simple
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_ToneMapping_Simple()));
	}
}

technique11 ToneMapping_Simple_Invert
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_ToneMapping_Simple_Invert()));
	}
}

technique11 ToneMapping_ACES
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_ToneMapping_ACES_Filmic()));
	}
}

technique11 ToneMapping_Reinhard
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_ToneMapping_Reinhard()));
	}
}