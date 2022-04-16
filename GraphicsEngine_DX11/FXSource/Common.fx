/**********************************************/
// 공통되는 Constant Buffer들은 한 곳에서 관리
/*********************************************/

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
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldView;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldInvTransposeView;
	float4x4 gWorldViewProj;
	float4x4 gViewProj;
	float4x4 gTexTransform;
	float4x4 gShadowTransform;
	Material gMaterial;
	float gAlpha;
	
	// PBR
	float gMetallic;
	float gRoughness;
};

// Nonnumeric values cannot be added to a cbuffer.
// (비수치 값들은 상수 버퍼에 추가할 수 없다.)
Texture2D gDiffuseMap;
Texture2D gNormalMap;
Texture2D gShadowMap;
Texture2D gEmissiveMap;
Texture2D gMetallicMap;
Texture2D gRoughnessMap;
TextureCube gCubeMap;

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

// MRT
struct PSOut
{
	float4 Albedo : SV_Target0;		// Color
	float4 Material : SV_Target1;	// Material
	float4 PosW : SV_Target2;		// Pos in World
	float4 PosH : SV_Target3;		// Pos in homogeneous (NDC).xy, Depth(z)
	float4 NormalW : SV_Target4;	// NormalW
	float4 ShadowPosH : SV_Target5;	// Shadow Pos in H
	float4 NormalDepth : SV_Target6;// NormalV, PosV.z
};
