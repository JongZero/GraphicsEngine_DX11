//=============================================================================
    // 텍스쳐 2개를 섞는다.
//=============================================================================
 
float gPer = 1.0f;		// 몇대몇의 비율로 섞을 것인지
float gFadePer;			// Fade In & Fade Out
float4x4  gWorldViewProj;

Texture2D gTexture_1;
Texture2D gTexture_2;
Texture2D gTexture_3;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
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

// MRT
struct PSOut
{
	float4 Original : SV_Target0;	// 원본
	float4 ForBlur : SV_Target1;	// Blur 먹일 렌더타겟
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
	float4 c_1 = float4(gTexture_1.Sample(samLinear, pin.Tex).xyz, 1.0f);
	float4 c_2 = float4(gTexture_2.Sample(samLinear, pin.Tex).xyz, 1.0f);
	
	float3 c = lerp(c_1.xyz, c_2.xyz, gPer);

	return float4(c.xyz, 1.0f);
}

float4 PS_Accumulate(VertexOut pin) : SV_Target
{
	float3 c_1 = float3(gTexture_1.Sample(samLinear, pin.Tex).xyz);
	float3 c_2 = float3(gTexture_2.Sample(samLinear, pin.Tex).xyz);

	c_2 *= gPer;

	float3 c = c_1 + c_2;

	return float4(c.xyz, 1.0f);
}

PSOut PS_MagicEye(VertexOut pin) : SV_Target
{
	float4 c_1 = float4(gTexture_1.Sample(samLinear, pin.Tex).xyz, 1.0f);
	float4 c_2 = float4(gTexture_2.Sample(samLinear, pin.Tex).xyz, 1.0f);

	//float3 c = lerp(c_1.xyz, c_2.xyz, 0.25f);

	c_1.xyz += c_2.xyz / 3;

	c_1.a *= 2 / 3;
	c_2.a *= 1 / 3;

	c_1.a += c_2.a;

	PSOut pOut;
	pOut.Original = float4(saturate(c_1.xyz), 1.0f);
	pOut.ForBlur = float4(saturate(c_1.xyz), 1.0f);
	return pOut;
}

PSOut PS_NotMagicEye(VertexOut pin) : SV_Target
{
	float4 c_1 = float4(gTexture_1.Sample(samLinear, pin.Tex).xyz, 1.0f);
	float4 c_2 = float4(gTexture_2.Sample(samLinear, pin.Tex).xyz, 1.0f);

	PSOut pOut;
	pOut.Original = float4(saturate(c_1.xyz + c_2.xyz), 1.0f);
	pOut.ForBlur = float4(saturate(c_1.xyz + c_2.xyz), 1.0f);
	return pOut;
}

float4 PS_FadeInOut(VertexOut pin) : SV_Target
{
	float4 c = float4(gTexture_1.Sample(samLinear, pin.Tex).xyz, 1.0f);

	return float4(c.xyz * gFadePer, 1.0f);
}

float4 PS_MixBloom(VertexOut pin) : SV_Target
{
	float4 c_1 = float4(gTexture_1.Sample(samLinear, pin.Tex).xyz, 1.0f);
	float4 c_2 = float4(gTexture_2.Sample(samLinear, pin.Tex).xyz, 1.0f);
	float4 c_3 = float4(gTexture_3.Sample(samLinear, pin.Tex).xyz, 1.0f);

	float4 bloom = pow(pow(abs(c_2), 2.2f) + pow(abs(c_3), 2.2f), 1.0f / 2.2f);

	c_1 = pow(abs(c_1), 2.2f);
	bloom = pow(abs(bloom), 2.2f);

	c_1 += bloom * gPer;

	c_1 = pow(abs(c_1), 1 / 2.2f);

	return float4(c_1.xyz, 1.0f);
}

technique11 MixTexture
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 MixTexture_Accumulate
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Accumulate()));
	}
}

technique11 MixTexture_MagicEye
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_MagicEye()));
	}
}

technique11 MixTexture_NotMagicEye
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_NotMagicEye()));
	}
}

technique11 MixTexture_FadeInOut
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_FadeInOut()));
	}
};

technique11 MixTexture_MixBloom
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_MixBloom()));
	}
};