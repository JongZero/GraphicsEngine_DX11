//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
// 
// 디퍼드로 커스텀, 감마 correction 추가
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

	// World Space로 변환한다.
	vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	//vout.NormalW = normalize(vout.NormalW);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// Output vertex attributes for interpolation across triangle.
	// 정점 값을 출력한다. 삼각형을 따라 보간된다.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	// 그림자 맵을 장면에 투영하기 위한 투영 텍스처 좌표를 생성한다.
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);

	return vout;
}

VertexOut_Instancing VS_Instancing(VertexIn_Instancing vin)
{
	VertexOut_Instancing vout;

	// Transform to view space
	vout.PosV = mul(float4(vin.PosL, 1.0f), gWorldView).xyz;
	vout.NormalV = mul(vin.NormalL, (float3x3)gWorldInvTransposeView);

	// World Space로 변환한다.
	vout.PosW = mul(float4(vin.PosL, 1.0f), vin.World).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)vin.World);
	//vout.NormalW = normalize(vout.NormalW);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vout.PosW, 1.0f), gViewProj);

	// Output vertex attributes for interpolation across triangle.
	// 정점 값을 출력한다. 삼각형을 따라 보간된다.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	vout.Color = vin.Color;

	// 그림자 맵을 장면에 투영하기 위한 투영 텍스처 좌표를 생성한다.
	vout.ShadowPosH = mul(float4(vin.PosL, 1.0f), gShadowTransform);

	return vout;
}

float4 PS_Forward(VertexOut pin, uniform bool gUseTexure) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	// 보간 때문에 노말벡터가 더 이상 단위벡터가 아닐 수 있으므로 다시 정규화한다.
	pin.NormalW = normalize(pin.NormalW);
	float3 originalNormal = pin.NormalW;

	// The toEye vector is used in lighting.
	// toEye 벡터는 조명 계산에 쓰인다.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	// 시점과 이 표면 점 사이의 거리를 보관해둔다.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;

	// Default to multiplicative identity.
	// 기본 텍스처 색상은 곱셈의 항등원
	float4 texColor = float4(1, 1, 1, 1);
	if (gUseTexure)
	{
		// Sample texture.
		// 텍스처에서 표본을 추출한다.
		texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
	}

	// Gamma Correction, 2.2f는 정확도, 2.0f는 속도에서 이점이 있다. 
	// Gamma Correction : Encoding된 데이터(텍스처)를 Linear 공간으로 불러와(2.2f 제곱을 하면 됨)
	// 라이트 연산을 하고 다시 1 / 2.2f 제곱을 하여 다시 본래 Encoding 공간으로 이동시킨다.
	// (모니터와 같은 광학장치는 Encoding 되어있는 이미지를 Decoding을 하여 화면에 이미지를 띄운다.)
	texColor = pow(texColor, 2.2f);

	// Lighting.

	float4 litColor = texColor;

	// 각 광원이 기여한 빛을 합한다.
	float4 A, D, S;

	// Start with a sum of zero. 
	// 성분들의 합이 0인 재질 속성들로 시작한다.
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (gDirLightCount > 0)
	{
		// 첫 번째 광원만 그림자를 드리운다.
		float3 shadow = float3(1.0f, 1.0f, 1.0f);
		//shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

		// Sum the light contribution from each light source.  
		// 각 광원이 기여한 빛을 합한다.
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
	// 분산광(Diffuse) 재질의 알파와 텍스쳐의 알파의 곱을 전체적인 알파 값으로 사용한다.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

	return litColor;
}

float4 PS_Forward_Instancing(VertexOut_Instancing pin, uniform bool gUseTexure) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
	// 보간 때문에 노말벡터가 더 이상 단위벡터가 아닐 수 있으므로 다시 정규화한다.
	pin.NormalW = normalize(pin.NormalW);
	float3 originalNormal = pin.NormalW;

	// The toEye vector is used in lighting.
	// toEye 벡터는 조명 계산에 쓰인다.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	// 시점과 이 표면 점 사이의 거리를 보관해둔다.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;

	// Default to multiplicative identity.
	// 기본 텍스처 색상은 곱셈의 항등원
	float4 texColor = float4(1, 1, 1, 1);
	if (gUseTexure)
	{
		// Sample texture.
		// 텍스처에서 표본을 추출한다.
		texColor = gDiffuseMap.Sample(samLinear, pin.Tex);
	}

	// Gamma Correction, 2.2f는 정확도, 2.0f는 속도에서 이점이 있다. 
	// Gamma Correction : Encoding된 데이터(텍스처)를 Linear 공간으로 불러와(2.2f 제곱을 하면 됨)
	// 라이트 연산을 하고 다시 1 / 2.2f 제곱을 하여 다시 본래 Encoding 공간으로 이동시킨다.
	// (모니터와 같은 광학장치는 Encoding 되어있는 이미지를 Decoding을 하여 화면에 이미지를 띄운다.)
	texColor = pow(texColor, 2.2f);

	// Lighting.

	float4 litColor = texColor;

	// 각 광원이 기여한 빛을 합한다.
	float4 A, D, S;

	// Start with a sum of zero. 
	// 성분들의 합이 0인 재질 속성들로 시작한다.
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	if (gDirLightCount > 0)
	{
		// 첫 번째 광원만 그림자를 드리운다.
		float3 shadow = float3(1.0f, 1.0f, 1.0f);
		//shadow[0] = CalcShadowFactor(samShadow, gShadowMap, pin.ShadowPosH);

		// Sum the light contribution from each light source.  
		// 각 광원이 기여한 빛을 합한다.
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
	// 분산광(Diffuse) 재질의 알파와 텍스쳐의 알파의 곱을 전체적인 알파 값으로 사용한다.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

	return float4(litColor.xyz, 1.0f);
}

PSOut PS_Deferred(VertexOut pin, uniform bool gUseTexure) : SV_Target
{
	PSOut pOut;

	// Interpolating normal can unnormalize it, so normalize it.
	// 보간 때문에 노말벡터가 더 이상 단위벡터가 아닐 수 있으므로 다시 정규화한다.
	pin.NormalW = normalize(pin.NormalW);
	pin.NormalV = normalize(pin.NormalV);

	// Default to multiplicative identity.
	// 기본 텍스처 색상은 곱셈의 항등원
	float4 texColor = float4(1, 1, 1, 1);
	if (gUseTexure)
	{
		// Sample texture.
		// 텍스처에서 표본을 추출한다.
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
