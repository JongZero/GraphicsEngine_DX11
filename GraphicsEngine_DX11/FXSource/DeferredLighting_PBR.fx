// Deferred Lighting
// 지연된 라이팅
// 각 쉐이더에서 MRT로 텍스처에 Pos, Normal, Shadow... 등등을 찍고 
// 이 곳에서 라이팅 연산을 한다.

#include "LightHelper.fx"

TextureCube gIrradianceMap;
TextureCube gPreFilterMap;
Texture2D gBrdfLUT;

static const float PI = 3.14159265359;

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    //cosTheta는 표면의 법선 n과 뷰 방향 v 사이의 내적 결과이다.
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH * NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	
	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	
	return num / denom;
}
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);
	
	return ggx1 * ggx2;
}

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

Texture2D gAlbedo; // Color
Texture2D gMaterialRT; // Material
Texture2D gPosW; // Pos in World
Texture2D gPosH; // Pos in homogeneous (NDC).xy, Depth(z)
Texture2D gNormalW; // NormalW
Texture2D gShadowPosH; // Shadow Pos in H
Texture2D gShadowMap; // Shadow Map
Texture2D gSSAOMap; // SSAO Map

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
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
	float3 albedo = gAlbedo.Sample(samLinear, pin.Tex);
	float4 shadowPixel = gShadowPosH.Sample(samLinear, pin.Tex);
	float4 material = gMaterialRT.Sample(samLinear, pin.Tex);
	float3 posW = gPosW.Sample(samLinear, pin.Tex);
	float3 posH = gPosH.Sample(samLinear, pin.Tex);
	float3 normalW = gNormalW.Sample(samLinear, pin.Tex);
	float4 ssaoPosH = mul(float4(posW, 1.0f), gViewProjTex);

	float3 WorldPos = posW;
	float3 camPos = gEyePosW;

	int lightIndex = 0;
	float4 lightPositions[4];
	float4 lightColours[4];
	for (int k = 0; k < 2; k++)
	{
		for (int j = 0; j < 2; j++)
		{
			lightPositions[lightIndex] = float4(2.5f + k * 10.0f, 12.5f + j * 10.0f, 10.0f, 0.0f);
			lightColours[lightIndex] = float4(300.0f, 300.0f, 300.0f, 0.0f);
			lightIndex++;
		}
	}
	
	albedo = pow(albedo, 2.2f);
	//albedo = float3(1, 0, 0);

	// 총 4가지 경우의 수
	// metallic map (X) 값으로 받음(posH.x), roughness map (X) 값으로 받음(posH.y)
	// metallic map (X) 값으로 받음(posH.x), roughness map (O) 맵사용
	// metallic map (O) 맵사용            , roughness map (X) 값으로 받음(posH.y)
	// metallic map (O) 맵사용            , roughness map (O) 맵사용
	// 이건 Basic.fx에서 결정할 것이고, 여기서는 그냥 받은 값 그대로 쓰면 된다.
	float3 Normal = normalW; // * normalMap.Sample(textureSampler, input.uv).rgb;
	float metallic = posH.x; // metallicMap.Sample(textureSampler, input.uv).r;
	float roughness = posH.y; // roughnessMap.Sample(textureSampler, input.uv).r;

	float ao = 1.0;

	float3 N = normalize(Normal);
	float3 V = normalize(camPos.xyz - WorldPos);

	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metallic);

    // reflectance equation
	float3 Lo = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < 1; ++i)
	{
		/*
        // calculate per-light radiance
		float3 L = normalize(lightPositions[i].xyz - WorldPos);
		float3 H = normalize(V + L);
		float distance = length(lightPositions[i].xyz - WorldPos);
		float attenuation = 1.0 / (distance * distance);
        //float attenuation = 10.0 / (distance);
		float3 radiance = lightColours[i].xyz * attenuation;
		*/

		float3 L = normalize(gDirLights[i].Direction);
		L *= -1;
		float3 H = normalize(V + L);
		float3 radiance = lightColours[i].xyz / 100;
		
        // cook-torrance brdf
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		float3 kS = F;
		float3 kD = float3(1.0, 1.0, 1.0) - kS;
		kD *= 1.0 - metallic;

		float3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
		float3 specular = numerator / max(denominator, 0.001); // 어떤 내적이 0으로 끝나는 경우 0으로 나누는 것을 막기위해 분모를 0.001로 제한한다.


        // add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	//float3 ambient = float3(0.03f, 0.03f, 0.03f) * albedo * ao;
	float3 ambient = float3(0.1f, 0.1f, 0.1f) * albedo * ao;
	
	float3 color = ambient + Lo;

	return float4(color, 1.0);
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
