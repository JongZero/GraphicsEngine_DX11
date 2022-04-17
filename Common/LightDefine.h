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

			/// Main Directional Light��� �׸��ڸ� �ϴ� �׸���.
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
/// C++�� ��ŷ ��Ģ�� HLSL�� �е� ��Ģ�� �ٸ��� ������ HLSL�� ��Ģ�� ��������Ѵ�.
/// HLSL������ �ڷ� ������� 4���� ���� ������ ä�� �ֵ� �ϳ��� ����� �� ���� 4���� ���Ϳ� ���� ����������
/// �� �ȴٴ� ������ ��Ű�� ���� ������ ����Ʈ���� �߰��ȴ�.

struct DirectionalLightData
{
	DirectionalLightData() { ZeroMemory(this, sizeof(this)); }

	EMath::FLOAT4 Ambient;
	EMath::FLOAT4 Diffuse;
	EMath::FLOAT4 Specular;
	EMath::FLOAT3 Direction;
	float Pad; // �ʿ��ϴٸ� Light���� �迭�� ������ �� �ֵ���, �������� float �� �ڸ��� ������ �д�.
};

struct PointLightData
{
	PointLightData() { ZeroMemory(this, sizeof(this)); }

	EMath::FLOAT4 Ambient;
	EMath::FLOAT4 Diffuse;
	EMath::FLOAT4 Specular;

	// (Position, Range) ���·� �ϳ��� 4���� ����(4D vector)�� ä�� �ִ´�.
	EMath::FLOAT3 Position;
	float Range;

	// (A0, A1, A2, Pad) ���·� �ϳ��� 4���� ����(4D vector)�� ä�� �ִ´�.
	EMath::FLOAT3 Att;
	float Pad; // �ʿ��ϴٸ� Light���� �迭�� ������ �� �ֵ���, �������� float �� �ڸ��� ������ �д�.
};

struct SpotLightData
{
	SpotLightData() { ZeroMemory(this, sizeof(this)); }

	EMath::FLOAT4 Ambient;
	EMath::FLOAT4 Diffuse;
	EMath::FLOAT4 Specular;

	// (Position, Range) ���·� �ϳ��� 4���� ����(4D vector)�� ä�� �ִ´�.
	EMath::FLOAT3 Position;
	float Range;

	// (Direction, Spot) ���·� �ϳ��� 4���� ����(4D vector)�� ä�� �ִ´�.
	EMath::FLOAT3 Direction;
	float Spot;

	// (Att, Pad) ���·� �ϳ��� 4���� ����(4D vector)�� ä�� �ִ´�.
	EMath::FLOAT3 Att;
	float Pad; // �ʿ��ϴٸ� Light���� �迭�� ������ �� �ֵ���, �������� float �� �ڸ��� ������ �д�.
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