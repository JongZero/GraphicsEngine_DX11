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
	// ��� ���е��� �ʱ�ȭ�Ѵ�.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	// �� ���ʹ� ������ ���ư��� ������ �ݴ� �����̴�.
	float3 lightVec = -L.Direction;

	// Add ambient term.
	// �ֺ���(ambient)�� ���Ѵ�.
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// ���� ������ �ʰ� ǥ�鿡 �����Ѵٴ� ���� �Ͽ���
	// �л걤(diffuse)�� �ݿ���(spec)�� ���Ѵ�.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// ���� �б⸦ ���ϱ� ���� ���ǹ��� ��ģ��.
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
	// ��� ���е��� �ʱ�ȭ�Ѵ�.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	// �� ���� : ǥ�� ������ ���������� ����
	float3 lightVec = L.Position - pos;
		
	// ǥ�� ���� ���� ������ �Ÿ�
	// The distance from surface to light.
	float d = length(lightVec);
	
	// ���� ����
	// Range test.
	//if( d > L.Range )
	//	return;
		
	// �� ���͸� ����ȭ�Ѵ�.
	// Normalize the light vector.
	lightVec /= d; 
	
	// �Ÿ��� ���� ���⸦ ���� ����Ѵ�.
	// ���Ⱑ ���� 0�̰ų� 0���϶�� �� �̻� ������� �ʴ´�.
	// Attenuate
	// ����
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

	if (att <= 0.0f)
	{
		return;
	}

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// ���� ������ �ʰ� ǥ�鿡 �����Ѵٴ� ���� �Ͽ���
	// �л걤(diffuse)�� �ݿ���(spec)�� ���Ѵ�.

	float diffuseFactor = saturate(dot(lightVec, normal));

	// Flatten to avoid dynamic branching.
	// ���� �б⸦ ���ϱ� ���� ���ǹ��� ��ģ��.
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

	// �ֺ���
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
	// ��� ���е��� �ʱ�ȭ�Ѵ�.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	// �� ���� : ǥ�� ������ ���������� ����
	float3 lightVec = L.Position - pos;
		
	// ǥ�� ���� ���� ������ �Ÿ�
	// The distance from surface to light.
	float d = length(lightVec);
	
	// ���� ����
	// Range test.
	if( d > L.Range )
		return;
		
	// �� ���͸� ����ȭ�Ѵ�.
	// Normalize the light vector.
	lightVec /= d; 
	
	// �ֺ���
	// Ambient term.
	ambient = mat.Ambient * L.Ambient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	// ���� ������ �ʰ� ǥ�鿡 �����Ѵٴ� ���� �Ͽ���
	// �л걤(diffuse)�� �ݿ���(spec)�� ���Ѵ�.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	// ���� �б⸦ ���ϱ� ���� ���ǹ��� ��ģ��.
	[flatten]
	if( diffuseFactor > 0.0f )
	{
		float3 v         = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
					
		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec    = specFactor * mat.Specular * L.Specular;
	}
	
	// Scale by spotlight factor and attenuate.
	// ������(Spot Light) ����� ����Ѵ�.
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// Scale by spotlight factor and attenuate.
	// ���� ���⸦ ������(Spot Light) ����� ����ϰ� �Ÿ��� ���� �����Ų��.
	float att = spot / dot(L.Att, float3(1.0f, d, d*d));

	ambient *= spot;
	diffuse *= att;
	spec    *= att;
}

// �븻 �� ������ World Space�� ��ȯ�Ѵ�.
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// �� ������ [0, 1]���� [-1, 1]�� ����Ѵ�.
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	// tangent Space ���� ���͸� �����.
	float3 N = unitNormalW;		// �븻����
	float3 T = normalize(tangentW - dot(tangentW, N) * N);		// Tangent
	float3 B = cross(N, T);		// Bitangent, �븻 ���Ϳ� ź��Ʈ�� �����Ͽ� ����
	
	float3x3 TBN = float3x3(T, B, N);

	// tangent Space���� World Space�� ��ȯ�Ѵ�.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

/***********************/
	// Shadow Map //
/***********************/
static const float SMAP_SIZE = 2048.0f;		// ������ �������� ��������� ������ �� �Ķ���Ͱ�
static const float SMAP_DX = 1.0f / SMAP_SIZE;

float CalcShadowFactor(SamplerComparisonState samShadow, Texture2D shadowMap, float4 shadowPosH)
{
	// w�� ����� ������ �Ϸ��Ѵ�.
	shadowPosH.xyz /= shadowPosH.w;

	// NDC ���� ������ ���� ��
	float depth = shadowPosH.z;

	// �ؼ� ũ��
	const float dx = SMAP_DX;

	float percentLit = 0.0f;
	const float2 offsets[9] =
	{
		float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
	};

	// 3x3���� ���� ����. �� ǥ������ 4ǥ�� PCF�� �����Ѵ�.
	[unroll]
	for (int i = 0; i < 9; ++i)
	{
		percentLit += shadowMap.SampleCmpLevelZero(samShadow,
			shadowPosH.xy + offsets[i], depth).r;
	}

	// ������� ����� ��ȯ�Ѵ�.
	return percentLit /= 9.0f;
}