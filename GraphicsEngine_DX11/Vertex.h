#pragma once

#include <d3d11.h>
#include "EMath.h"

namespace Vertex
{
	struct PosColor
	{
		EMath::FLOAT3 Pos;
		EMath::FLOAT4 Color;
	};

	struct PosNormalTex
	{
		EMath::FLOAT3 Pos;
		EMath::FLOAT3 Normal;
		EMath::FLOAT2 Tex;
	};

	// Texture, Normal Map
	struct PosNormalTexTangentU
	{
		EMath::FLOAT3 Pos;
		EMath::FLOAT3 Normal;
		EMath::FLOAT2 Tex;
		EMath::FLOAT3 TangentU;
	};

	// Skinning
	struct PosNormalTexTangentUWeightsBoneIndices
	{
		EMath::FLOAT3 Pos;
		EMath::FLOAT3 Normal;
		EMath::FLOAT2 Tex;
		EMath::FLOAT3 TangentU;
		EMath::FLOAT3 Weights;
		BYTE BoneIndices[4] = { 0, };
	};

	// Instancing
	struct InstancedData
	{
		EMath::FLOAT4X4 World;
		EMath::FLOAT4 Color;
	};

	struct Particle
	{
		EMath::FLOAT3 InitialPos;
		EMath::FLOAT3 InitialVel;
		EMath::FLOAT2 Size;
		float Age;
		unsigned int Type;
	};
}

class InputLayoutDesc
{
public:
	// Init like const int A::a[4] = {0, 1, 2, 3}; in .cpp file.
	static const D3D11_INPUT_ELEMENT_DESC PosColor[2];
	static const D3D11_INPUT_ELEMENT_DESC PosNormalTex[3];
	static const D3D11_INPUT_ELEMENT_DESC InstancedPosNormalTex[8];
	static const D3D11_INPUT_ELEMENT_DESC PosNormalTexTangentU[4];
	static const D3D11_INPUT_ELEMENT_DESC PosNormalTexTangentUWeightsBoneIndices[6];
	static const D3D11_INPUT_ELEMENT_DESC Particle[5];
};

class InputLayouts
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11InputLayout* PosColor;
	static ID3D11InputLayout* PosNormalTex;
	static ID3D11InputLayout* InstancedPosNormalTex;
	static ID3D11InputLayout* PosNormalTexTangentU;
	static ID3D11InputLayout* PosNormalTexTangentUWeightsBoneIndices;
	static ID3D11InputLayout* Particle;
};