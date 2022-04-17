#pragma once
#include <windows.h>
#include "EMath.h"

struct LegacyMaterialData
{
	LegacyMaterialData() { ZeroMemory(this, sizeof(this)); }

	EMath::Vector4 Ambient;
	EMath::Vector4 Diffuse;
	EMath::Vector4 Specular; // w = SpecPower
	EMath::Vector4 Reflect;
};

class LightInfo
{
public:
	LightInfo() {}
	virtual ~LightInfo() {}

public:
	EMath::Vector4 Ambient;
	EMath::Vector4 Diffuse;
	EMath::Vector4 Specular;

	bool IsCastShadow = false;
};

class LightPositionInfo : public LightInfo
{
public:
	LightPositionInfo() {}
	virtual ~LightPositionInfo() {}

public:
	EMath::Vector3 Position;
	float Range = 0.0f;
};

class DirectionalLightInfo : public LightInfo
{
public:
	enum class eType
	{
		Main,
		Side,
		Back,
	};

public:
	DirectionalLightInfo(eType type)
		: Direction({ 0, 0, 0 })
	{
		switch (type)
		{
		case DirectionalLightInfo::eType::Main:
			Ambient = EMath::Vector4(0.3f, 0.3f, 0.3f, 1.0f);
			Diffuse = EMath::Vector4(0.75f, 0.75f, 0.75f, 1.0f);
			Specular = EMath::Vector4(0.5f, 0.5f, 0.5f, 1.0f);
			//Direction = EMath::Vector3(0.57735f, -0.57735f, 0.57735f);

			Direction = EMath::Vector3(0.0f, -0.707f, -0.707f);
			//Direction = EMath::Vector3(-0.57735f, -0.57735f, 0.57735f);

			/// Main Directional Light라면 그림자를 일단 그린다.
			IsCastShadow = true;
			
			break;
		case DirectionalLightInfo::eType::Side:
			Ambient = EMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			Diffuse = EMath::Vector4(0.20f, 0.20f, 0.20f, 1.0f);
			Specular = EMath::Vector4(0.25f, 0.25f, 0.25f, 1.0f);
			Direction = EMath::Vector3(-0.57735f, -0.57735f, 0.57735f);
			break;
		case DirectionalLightInfo::eType::Back:
			Ambient = EMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			Diffuse = EMath::Vector4(0.2f, 0.2f, 0.2f, 1.0f);
			Specular = EMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
			Direction = EMath::Vector3(0.0f, -0.707f, -0.707f);
			break;
		}
	}
	virtual ~DirectionalLightInfo() {}

public:
	EMath::Vector3 Direction;
};

class PointLightInfo : public LightPositionInfo
{
public:
	PointLightInfo()
	{
		Ambient = EMath::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		Diffuse = EMath::Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		Specular = EMath::Vector4(0.1f, 0.1f, 0.1f, 1.0f);
		Att = EMath::Vector3(0.0f, 0.5f, 0.0f);

		Range = 1.0f;

		Position = EMath::Vector3(0, 0, 0);
	}
	virtual ~PointLightInfo() {}

public:
	EMath::Vector3 Att;
};

class SpotLightInfo : public LightPositionInfo
{
public:
	SpotLightInfo()
	{
		Ambient = EMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		Diffuse = EMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		Specular = EMath::Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		Att = EMath::Vector3(1.0f, 0.0f, 0.0f);

		Spot = 96.0f;
		Range = 100000.0f;

		Position = EMath::Vector3(0, 0, 0);
		Direction = EMath::Vector3(0, 0, 0);
	}
	virtual ~SpotLightInfo() {}

public:
	EMath::Vector3 Direction;
	float Spot;

	EMath::Vector3 Att;
};

#ifdef GRAPHICSENGINE_DX11_EXPORTS

// Note: Make sure structure alignment agrees with HLSL structure padding rules.
//   Elements are packed into 4D vectors with the restriction that an element
//   cannot straddle a 4D vector boundary.
/// C++의 패킹 규칙과 HLSL의 패딩 규칙이 다르기 때문에 HLSL의 규칙에 맞춰줘야한다.
/// HLSL에서는 자료 멤버들을 4차원 벡터 단위로 채워 넣되 하나의 멤버가 두 개의 4차원 벡터에 걸쳐 나누어지면
/// 안 된다는 제약을 지키기 위해 여분의 바이트들이 추가된다.

struct DirectionalLightData
{
	DirectionalLightData() { ZeroMemory(this, sizeof(this)); }

	EMath::FLOAT4 Ambient;
	EMath::FLOAT4 Diffuse;
	EMath::FLOAT4 Specular;
	EMath::FLOAT3 Direction;
	float Pad; // 필요하다면 Light들의 배열을 설정할 수 있도록, 마지막에 float 한 자리를 마련해 둔다.
};

struct PointLightData
{
	PointLightData() { ZeroMemory(this, sizeof(this)); }

	EMath::FLOAT4 Ambient;
	EMath::FLOAT4 Diffuse;
	EMath::FLOAT4 Specular;

	// (Position, Range) 형태로 하나의 4차원 벡터(4D vector)에 채워 넣는다.
	EMath::FLOAT3 Position;
	float Range;

	// (A0, A1, A2, Pad) 형태로 하나의 4차원 벡터(4D vector)에 채워 넣는다.
	EMath::FLOAT3 Att;
	float Pad; // 필요하다면 Light들의 배열을 설정할 수 있도록, 마지막에 float 한 자리를 마련해 둔다.
};

struct SpotLightData
{
	SpotLightData() { ZeroMemory(this, sizeof(this)); }

	EMath::FLOAT4 Ambient;
	EMath::FLOAT4 Diffuse;
	EMath::FLOAT4 Specular;

	// (Position, Range) 형태로 하나의 4차원 벡터(4D vector)에 채워 넣는다.
	EMath::FLOAT3 Position;
	float Range;

	// (Direction, Spot) 형태로 하나의 4차원 벡터(4D vector)에 채워 넣는다.
	EMath::FLOAT3 Direction;
	float Spot;

	// (Att, Pad) 형태로 하나의 4차원 벡터(4D vector)에 채워 넣는다.
	EMath::FLOAT3 Att;
	float Pad; // 필요하다면 Light들의 배열을 설정할 수 있도록, 마지막에 float 한 자리를 마련해 둔다.
};

struct LegacyMaterialData_Converted
{
	LegacyMaterialData_Converted() { ZeroMemory(this, sizeof(this)); }

	EMath::FLOAT4 Ambient;
	EMath::FLOAT4 Diffuse;
	EMath::FLOAT4 Specular; // w = SpecPower
	EMath::FLOAT4 Reflect;
};

#endif