//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
// 
// ���۵�� Ŀ����, ���� correction �߰�
//=============================================================================

#include "Common.fx"

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexIn_Instancing
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
	row_major float4x4 World : WORLD;
	float4 Color : COLOR;
	uint InstanceId : SV_InstanceID;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float3 PosV    : POSITION0;
	float3 PosW    : POSITION1;
	float3 NormalV : NORMAL0;
	float3 NormalW : NORMAL1;
	float2 Tex     : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
};

struct VertexOut_Instancing
{
	float4 PosH    : SV_POSITION;
	float3 PosV    : POSITION0;
	float3 PosW    : POSITION1;
	float3 NormalV : NORMAL0;
	float3 NormalW : NORMAL1;
	float2 Tex     : TEXCOORD0;
	float4 ShadowPosH : TEXCOORD1;
	float4 Color : COLOR;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Transform to view space
	vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView).xyz;
	vout.NormalV = mul(vin.NormalL, (float3x3)gWorldInvTransposeView);

	// World Space�� ��ȯ�Ѵ�.
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	//vout.NormalW = normalize(vout.NormalW);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// Output vertex attributes for interpolation across triangle.
	// ���� ���� ����Ѵ�. �ﰢ���� ���� �����ȴ�.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	// �׸��� ���� ��鿡 �����ϱ� ���� ���� �ؽ�ó ��ǥ�� �����Ѵ�.
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);

	return vout;
}

VertexOut_Instancing VS_Instancing(VertexIn_Instancing vin)
{
	VertexOut_Instancing vout;

	// Transform to view space
	vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView).xyz;
	vout.NormalV = mul(vin.NormalL, (float3x3)gWorldInvTransposeView);

	// World Space�� ��ȯ�Ѵ�.
	vout.PosW = mul(float4(vin.PosL, 1.0f), vin.World).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)vin.World);
	//vout.NormalW = normalize(vout.NormalW);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);

	// Output vertex attributes for interpolation across triangle.
	// ���� ���� ����Ѵ�. �ﰢ���� ���� �����ȴ�.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	vout.Color = vin.Color;

	// �׸��� ���� ��鿡 �����ϱ� ���� ���� �ؽ�ó ��ǥ�� �����Ѵ�.
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);

	return vout;
}

float4 PS_Forward(VertexOut pin, uniform bool gUseTexure) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	// ���� ������ �븻���Ͱ� �� �̻� �������Ͱ� �ƴ� �� �����Ƿ� �ٽ� ����ȭ�Ѵ�.
	pin.NormalW = normalize(pin.NormalW);
	float3 originalNormal = pin.NormalW;

	// The toEye vector is used in lighting.
	// toEye ���ʹ� ���� ��꿡 ���δ�.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	// ������ �� ǥ�� �� ������ �Ÿ��� �����صд�.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;

	// Default to multiplicative identity.
	// �⺻ �ؽ�ó ������ ������ �׵��
	float4 texColor = float4(1, 1, 1, 1);
	if (gUseTexure)
	{
		// Sample texture.
		// �ؽ�ó���� ǥ���� �����Ѵ�.
		texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
	}

	// Gamma Correction, 2.2f�� ��Ȯ��, 2.0f�� �ӵ����� ������ �ִ�. 
	// Gamma Correction : Encoding�� ������(�ؽ�ó)�� Linear �������� �ҷ���(2.2f ������ �ϸ� ��)
	// ����Ʈ ������ �ϰ� �ٽ� 1 / 2.2f ������ �Ͽ� �ٽ� ���� Encoding �������� �̵���Ų��.
	// (����Ϳ� ���� ������ġ�� Encoding �Ǿ��ִ� �̹����� Decoding�� �Ͽ� ȭ�鿡 �̹����� ����.)
	texColor = pow(texColor, 2.2f);

	// Lighting.

	float4 litColor = texColor;

	// �� ������ �⿩�� ���� ���Ѵ�.
	float4 A, D, S;

	// Start with a sum of zero. 
	// ���е��� ���� 0�� ���� �Ӽ���� �����Ѵ�.
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (gDirLightCount > 0)
	{
		// ù ��° ������ �׸��ڸ� �帮���.
		float3 shadow = float3(1.0f, 1.0f, 1.0f);
		//shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

		// Sum the light contribution from each light source.  
		// �� ������ �⿩�� ���� ���Ѵ�.
		[unroll]
		for (int i = 0; i < gDirLightCount; ++i)
		{
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye,
				A, D, S);

			ambient += A;
			diffuse += shadow[i] * D;
			spec += shadow[i] * S;
		}
	}

	for (int i = 0; i < gPointLightCount; ++i)
	{
		ComputePointLight(gMaterial, gPointLights[i], pin.PosW, pin.NormalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}

	for (int i = 0; i < gSpotLightCount; ++i)
	{
		ComputeSpotLight(gMaterial, gSpotLight[i], pin.PosW, pin.NormalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}

	// Modulate with late add.
	litColor = texColor * (ambient + diffuse) + spec;

	// Common to take alpha from diffuse material and texture.
	// �л걤(Diffuse) ������ ���Ŀ� �ؽ����� ������ ���� ��ü���� ���� ������ ����Ѵ�.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

	return litColor;
}

float4 PS_Forward_Instancing(VertexOut_Instancing pin, uniform bool gUseTexure) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	// ���� ������ �븻���Ͱ� �� �̻� �������Ͱ� �ƴ� �� �����Ƿ� �ٽ� ����ȭ�Ѵ�.
	pin.NormalW = normalize(pin.NormalW);
	float3 originalNormal = pin.NormalW;

	// The toEye vector is used in lighting.
	// toEye ���ʹ� ���� ��꿡 ���δ�.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	// ������ �� ǥ�� �� ������ �Ÿ��� �����صд�.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;

	// Default to multiplicative identity.
	// �⺻ �ؽ�ó ������ ������ �׵��
	float4 texColor = float4(1, 1, 1, 1);
	if (gUseTexure)
	{
		// Sample texture.
		// �ؽ�ó���� ǥ���� �����Ѵ�.
		texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
	}

	// Gamma Correction, 2.2f�� ��Ȯ��, 2.0f�� �ӵ����� ������ �ִ�. 
	// Gamma Correction : Encoding�� ������(�ؽ�ó)�� Linear �������� �ҷ���(2.2f ������ �ϸ� ��)
	// ����Ʈ ������ �ϰ� �ٽ� 1 / 2.2f ������ �Ͽ� �ٽ� ���� Encoding �������� �̵���Ų��.
	// (����Ϳ� ���� ������ġ�� Encoding �Ǿ��ִ� �̹����� Decoding�� �Ͽ� ȭ�鿡 �̹����� ����.)
	texColor = pow(texColor, 2.2f);

	// Lighting.

	float4 litColor = texColor;

	// �� ������ �⿩�� ���� ���Ѵ�.
	float4 A, D, S;

	// Start with a sum of zero. 
	// ���е��� ���� 0�� ���� �Ӽ���� �����Ѵ�.
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (gDirLightCount > 0)
	{
		// ù ��° ������ �׸��ڸ� �帮���.
		float3 shadow = float3(1.0f, 1.0f, 1.0f);
		//shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

		// Sum the light contribution from each light source.  
		// �� ������ �⿩�� ���� ���Ѵ�.
		[unroll]
		for (int i = 0; i < gDirLightCount; ++i)
		{
			ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye,
				A, D, S);

			ambient += A * pin.Color;
			diffuse += shadow[i] * D * pin.Color;
			spec += shadow[i] * S;
		}
	}

	for (int i = 0; i < gPointLightCount; ++i)
	{
		ComputePointLight(gMaterial, gPointLights[i], pin.PosW, pin.NormalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}

	for (int s = 0; s < gSpotLightCount; ++s)
	{
		ComputeSpotLight(gMaterial, gSpotLight[s], pin.PosW, pin.NormalW, toEye, A, D, S);
		ambient += A;
		diffuse += D;
		spec += S;
	}

	// Modulate with late add.
	litColor = texColor * (ambient + diffuse) + spec;

	// Common to take alpha from diffuse material and texture.
	// �л걤(Diffuse) ������ ���Ŀ� �ؽ����� ������ ���� ��ü���� ���� ������ ����Ѵ�.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

	return float4(litColor.xyz, 1.0f);
}

PSOut PS_Deferred(VertexOut pin, uniform bool gUseTexure) : SV_Target
{
	PSOut pOut;

	// Interpolating normal can unnormalize it, so normalize it.
	// ���� ������ �븻���Ͱ� �� �̻� �������Ͱ� �ƴ� �� �����Ƿ� �ٽ� ����ȭ�Ѵ�.
	pin.NormalW = normalize(pin.NormalW);
	pin.NormalV = normalize(pin.NormalV);

	// Default to multiplicative identity.
	// �⺻ �ؽ�ó ������ ������ �׵��
	float4 texColor = float4(1, 1, 1, 1);
	if (gUseTexure)
	{
		// Sample texture.
		// �ؽ�ó���� ǥ���� �����Ѵ�.
		texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
	}

	// Modulate with late add.
	float4 litColor = texColor;

	float shadow = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);
	
	pOut.Albedo = litColor;

	float ambient = gMaterial.Ambient.x;
	float diffuse = gMaterial.Diffuse.x;
	float specular = gMaterial.Specular.x;
	float specularPower = gMaterial.Specular.w;

	pOut.Material = float4(ambient, diffuse, specular, 1);	// gMaterial.Reflect.x

	pOut.PosW = float4(pin.PosW.xyz, 1.0f);

	float metallic = gMetallic;
	if(metallic < 0)
	{
		metallic = gMetallicMap.Sample(samLinear, pin.Tex).x;
	}
	
	float roughness = gRoughness;
	if(roughness < 0)
	{
		roughness = gRoughnessMap.Sample(samLinear, pin.Tex).x;
	}
	
	pOut.PosH = float4(metallic, roughness, pin.PosH.z, 1.0f);

	pOut.NormalW = float4(pin.NormalW, 1.0f);

	pOut.ShadowPosH = float4(shadow, specularPower, pin.PosV.z, 1);
	 
	pOut.NormalDepth = float4(pin.NormalV.xyz, 1.0f);

	return pOut;
}

technique11 Forward
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Forward(false)));
	}
}

technique11 ForwardTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Forward(true)));
	}
}

technique11 Forward_Instancing
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Instancing()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Forward_Instancing(false)));
	}
}

technique11 ForwardTex_Instancing
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Instancing()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Forward_Instancing(true)));
	}
}

// Deferred
technique11 Deferred
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Deferred(false)));
	}
}

technique11 DeferredTex
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Deferred(true)));
	}
}