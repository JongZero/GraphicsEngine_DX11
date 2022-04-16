// Deferred Lighting
// 지연된 라이팅
// 각 쉐이더에서 MRT로 텍스처에 Pos, Normal, Shadow... 등등을 찍고 
// 이 곳에서 라이팅 연산을 한다.

#include "LightHelper.fx"

cbuffer cbPerFrame
{
	DirectionalLight gDirLights[3];
	int gDirLightCount;
	PointLight gPointLights[50];
	int gPointLightCount;
	SpotLight gSpotLight[10];
	int gSpotLightCount;
	float3 gEyePosW;
	float4x4 gViewProjTex;
};

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

SamplerComparisonState samShadow
{
	Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU = BORDER;
	AddressV = BORDER;
	AddressW = BORDER;
	BorderColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	ComparisonFunc = LESS_EQUAL;
};

Texture2D gAlbedo;		// Color
Texture2D gMaterialRT;	// Material
Texture2D gPosW;		// Pos in World
Texture2D gPosH;		// Pos in homogeneous (NDC).xy, Depth(z)
Texture2D gNormalW;		// NormalW
Texture2D gShadowPosH;	// Shadow Pos in H
Texture2D gShadowMap;	// Shadow Map
Texture2D gSSAOMap;		// SSAO Map

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
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS(VertexOut pin, uniform bool gIsShadowActive, uniform bool gIsSsaoActive) : SV_Target
{
	float4 albedo = gAlbedo.Sample(samLinear, pin.Tex);
	float4 shadowPixel = gShadowPosH.Sample(samLinear, pin.Tex);
	float4 material = gMaterialRT.Sample(samLinear, pin.Tex);
	float3 posW = gPosW.Sample(samLinear, pin.Tex);
	float posH = gPosH.Sample(samLinear, pin.Tex).x;
	float3 normalW = gNormalW.Sample(samLinear, pin.Tex);
	float4 ssaoPosH = mul(float4(posW, 1.0f), gViewProjTex);

	// The toEye vector is used in lighting.
	// toEye 벡터는 조명 계산에 쓰인다.
	float3 toEye = gEyePosW - posW;

	// Cache the distance to the eye from this surface point.
	// 시점과 이 표면 점 사이의 거리를 보관해둔다.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
	// Gamma Correction, 2.2f는 정확도, 2.0f는 속도에서 이점이 있다.
	// Gamma Correction : Encoding된 데이터(텍스처)를 Linear 공간으로 불러와(2.2f 제곱을 하면 됨)
	// 라이트 연산을 하고 다시 1 / 2.2f 제곱을 하여 다시 본래 Encoding 공간으로 이동시킨다.
	// (모니터와 같은 광학장치는 Encoding 되어있는 이미지를 Decoding을 하여 화면에 이미지를 띄운다.)
	albedo = pow(albedo, 2.2f);

	// Lighting.

	float4 litColor = albedo;

	// 각 광원이 기여한 빛을 합한다.
	float4 A, D, S;

	// Start with a sum of zero.
	// 성분들의 합이 0인 재질 속성들로 시작한다.
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// 이 픽셀의 Material 수치
	Material mat;
	mat.Ambient = float4(material.xxx, 1.0f);
	mat.Diffuse = float4(material.yyy, 1.0f);
	mat.Specular = float4(material.zzz, shadowPixel.y);	// w는 specular의 power (1.0f가 아님)
	mat.Reflect = float4(0, 0, 0, 0);
	
	// Shadow
	float shadow = 1;
	
	if (gIsShadowActive)
		shadow = shadowPixel.x;

	float ambientAccess = 1;

	// 텍스쳐 투영을 통해서 SSAO 맵의 표본을 추출한다.
	if (gIsSsaoActive)
	{
		ssaoPosH /= ssaoPosH.w;
		ambientAccess = gSSAOMap.SampleLevel(samLinear, ssaoPosH.xy, 0.0f).r;
	}

	if (gDirLightCount > 0)
	{
		// 각 광원이 기여한 빛을 합한다.
		[unroll]
		for (int i = 0; i < gDirLightCount; ++i)
		{
			ComputeDirectionalLight(mat, gDirLights[i], normalW, toEye,
				A, D, S);

			// 추출한 도달도로 주변광 항을 변조한다.
			//ambient += ambientAccess * A;
			//diffuse += shadow * D;
			//spec += shadow * S;
			
			ambient += A;
			diffuse += D;
			spec += S;
		}
	}

	for (int i = 0; i < gPointLightCount; ++i)
	{
		ComputePointLight(mat, gPointLights[i], posW, normalW, toEye, A, D, S);
		
		ambient += ambientAccess * A;
		diffuse += shadow * D;
		spec += shadow * S;
	}
	
	for (int i = 0; i < gSpotLightCount; ++i)
	{
		ComputeSpotLight(mat, gSpotLight[i], posW, normalW, toEye, A, D, S);
		ambient += ambientAccess * A;
		diffuse += shadow * D;
		spec += shadow * S;
	}

	// Modulate with late add.
	litColor = albedo * (ambient + diffuse) + spec;

	// Common to take alpha from diffuse material and texture.
	// 분산광(Diffuse) 재질의 알파와 텍스쳐의 알파의 곱을 전체적인 알파 값으로 사용한다.
	litColor.a = mat.Diffuse.a * albedo.a;

	return litColor;
}

technique11 Lighting
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false, false)));
	}
}

technique11 LightingShadow
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(true, false)));
	}
}

technique11 LightingSSAO
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(false, true)));
	}
}

technique11 LightingShadowSSAO
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS(true, true)));
	}
}
