//***************************************************************************************
// LightHelper.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Structures and functions for lighting calculations.
//***************************************************************************************

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{ 
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
	float4 Reflect;
};

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(Material mat, DirectionalLight L, 
                             float3 normal, float3 toEye,
					         out float4 ambient,
						     out float4 diffuse,
						     out float4 spec)
{
	// Initialize outputs.
	// 출력 성분들을 초기화한다.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	// 빛 벡터는 광선이 나아가는 방향의 반대 방향이다.
	float3 lightVec = -L.Direction;

	// Add ambient term.
	// 주변광(ambient)을 더한다.
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// 빛이 막히지 않고 표면에 도달한다는 가정 하에서
	// 분산광(diffuse)과 반영광(spec)을 더한다.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// 동적 분기를 피하기 위해 조건문을 펼친다.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
				   out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	// 출력 성분들을 초기화한다.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	// 빛 벡터 : 표면 점에서 광원으로의 벡터
	float3 lightVec = L.Position - pos;
		
	// 표면 점과 광원 사이의 거리
	// The distance from surface to light.
	float d = length(lightVec);
	
	// 범위 판정
	// Range test.
	//if( d > L.Range )
	//	return;
		
	// 빛 벡터를 정규화한다.
	// Normalize the light vector.
	lightVec /= d; 
	
	// 거리에 따른 감쇄를 먼저 계산한다.
	// 감쇄가 만약 0이거나 0이하라면 더 이상 계산하지 않는다.
	// Attenuate
	// 감쇠
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

	if (att <= 0.0f)
	{
		return;
	}

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// 빛이 막히지 않고 표면에 도달한다는 가정 하에서
	// 분산광(diffuse)과 반영광(spec)을 더한다.

	float diffuseFactor = saturate(dot(lightVec, normal));

	// Flatten to avoid dynamic branching.
	// 동적 분기를 피하기 위해 조건문을 펼친다.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}

	diffuse *= att;
	spec    *= att;

	// 주변광
	// Ambient term.
	ambient = mat.Ambient * L.Ambient * att;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
				  out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	// 출력 성분들을 초기화한다.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	// 빛 벡터 : 표면 점에서 광원으로의 벡터
	float3 lightVec = L.Position - pos;
		
	// 표면 점과 광원 사이의 거리
	// The distance from surface to light.
	float d = length(lightVec);
	
	// 범위 판정
	// Range test.
	if( d > L.Range )
		return;
		
	// 빛 벡터를 정규화한다.
	// Normalize the light vector.
	lightVec /= d; 
	
	// 주변광
	// Ambient term.
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// 빛이 막히지 않고 표면에 도달한다는 가정 하에서
	// 분산광(diffuse)과 반영광(spec)을 더한다.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// 동적 분기를 피하기 위해 조건문을 펼친다.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
	
	// Scale by spotlight factor and attenuate.
	// 점적광(Spot Light) 계수를 계산한다.
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// Scale by spotlight factor and attenuate.
	// 빛의 세기를 점적광(Spot Light) 계수로 비례하고 거리에 따라 감쇠시킨다.
	float att = spot / dot(L.Att, float3(1.0f, d, d*d));

	ambient *= spot;
	diffuse *= att;
	spec    *= att;
}

// 노말 맵 샘플을 World Space로 변환한다.
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// 각 성분을 [0, 1]에서 [-1, 1]로 사상한다.
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	// tangent Space 기저 벡터를 만든다.
	float3 N = unitNormalW;		// 노말벡터
	float3 T = normalize(tangentW - dot(tangentW, N) * N);		// Tangent
	float3 B = cross(N, T);		// Bitangent, 노말 벡터와 탄젠트를 외적하여 구함
	
	float3x3 TBN = float3x3(T, B, N);

	// tangent Space에서 World Space로 변환한다.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

/***********************/
	// Shadow Map //
/***********************/
static const float SMAP_SIZE = 2048.0f;		// 렌더링 엔진에서 쉐도우맵을 생성할 때 파라미터값
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow, Texture2D shadowMap, float4 shadowPosH)
{
	// w로 나누어서 투영을 완료한다.
	shadowPosH.xyz /= shadowPosH.w;

	// NDC 공간 기준의 깊이 값
	float depth = shadowPosH.z;

	// 텍셀 크기
	const float dx = SMAP_DX;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	// 3x3상자 필터 패턴. 각 표본마다 4표본 PCF를 수행한다.
	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}

	// 결과들의 평균을 반환한다.
	return percentLit /= 9.0f;
}