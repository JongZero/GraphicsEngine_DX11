// Deferred Lighting
// ������ ������
// �� ���̴����� MRT�� �ؽ�ó�� Pos, Normal, Shadow... ����� ��� 
// �� ������ ������ ������ �Ѵ�.

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
	// toEye ���ʹ� ���� ��꿡 ���δ�.
	float3 toEye = gEyePosW - posW;

	// Cache the distance to the eye from this surface point.
	// ������ �� ǥ�� �� ������ �Ÿ��� �����صд�.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
	// Gamma Correction, 2.2f�� ��Ȯ��, 2.0f�� �ӵ����� ������ �ִ�.
	// Gamma Correction : Encoding�� ������(�ؽ�ó)�� Linear �������� �ҷ���(2.2f ������ �ϸ� ��)
	// ����Ʈ ������ �ϰ� �ٽ� 1 / 2.2f ������ �Ͽ� �ٽ� ���� Encoding �������� �̵���Ų��.
	// (����Ϳ� ���� ������ġ�� Encoding �Ǿ��ִ� �̹����� Decoding�� �Ͽ� ȭ�鿡 �̹����� ����.)
	albedo = pow(albedo, 2.2f);

	// Lighting.

	float4 litColor = albedo;

	// �� ������ �⿩�� ���� ���Ѵ�.
	float4 A, D, S;

	// Start with a sum of zero.
	// ���е��� ���� 0�� ���� �Ӽ���� �����Ѵ�.
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// �� �ȼ��� Material ��ġ
	Material mat;
	mat.Ambient = float4(material.xxx, 1.0f);
	mat.Diffuse = float4(material.yyy, 1.0f);
	mat.Specular = float4(material.zzz, shadowPixel.y);	// w�� specular�� power (1.0f�� �ƴ�)
	mat.Reflect = float4(0, 0, 0, 0);
	
	// Shadow
	float shadow = 1;
	
	if (gIsShadowActive)
		shadow = shadowPixel.x;

	float ambientAccess = 1;

	// �ؽ��� ������ ���ؼ� SSAO ���� ǥ���� �����Ѵ�.
	if (gIsSsaoActive)
	{
		ssaoPosH /= ssaoPosH.w;
		ambientAccess = gSSAOMap.SampleLevel(samLinear, ssaoPosH.xy, 0.0f).r;
	}

	if (gDirLightCount > 0)
	{
		// �� ������ �⿩�� ���� ���Ѵ�.
		[unroll]
		for (int i = 0; i < gDirLightCount; ++i)
		{
			ComputeDirectionalLight(mat, gDirLights[i], normalW, toEye,
				A, D, S);

			// ������ ���޵��� �ֺ��� ���� �����Ѵ�.
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
	// �л걤(Diffuse) ������ ���Ŀ� �ؽ����� ������ ���� ��ü���� ���� ������ ����Ѵ�.
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
