#include "Vertex.h"
#include "Effects.h"
#include "ErrChecker.h"

#include <MacroDefine.h>

#pragma region InputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosColor[2] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormalTex[3] =
{
	{"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::InstancedPosNormalTex[8] =
{
	{"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0},
	{ "WORLD",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "WORLD",		1, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 16,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "WORLD",		2, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 32,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "WORLD",		3, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 48,	D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 64,  D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormalTexTangentU[4] =
{
	{"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::PosNormalTexTangentUWeightsBoneIndices[6] =
{
	{"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,	0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"WEIGHTS",     0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"BONEINDICES", 0, DXGI_FORMAT_R8G8B8A8_UINT,	0, 56, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Particle[5] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
};
#pragma endregion

#pragma region InputLayouts
ID3D11InputLayout* InputLayouts::PosColor = 0;
ID3D11InputLayout* InputLayouts::PosNormalTex = 0;
ID3D11InputLayout* InputLayouts::InstancedPosNormalTex = 0;
ID3D11InputLayout* InputLayouts::PosNormalTexTangentU = 0;
ID3D11InputLayout* InputLayouts::PosNormalTexTangentUWeightsBoneIndices = 0;
ID3D11InputLayout* InputLayouts::Particle = 0;

void InputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

	// PosColor
	Effects::ColorFX->ColorTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::PosColor, 2, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosColor));

	// Pos, Normal, Texture
	Effects::BasicFX->ForwardTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::PosNormalTex, 3, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosNormalTex));

	// Instanced Pos, Normal, Tex
	Effects::BasicFX->Forward_InstancingTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::InstancedPosNormalTex, 8, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &InstancedPosNormalTex));

	// Pos, Normal, Texture, NormalMap
	Effects::NormalMapFX->ForwardTexNormalMapTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::PosNormalTexTangentU, 4, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosNormalTexTangentU));

	// Pos, Normal, Texture + Weights, BoneIndices, NormalMap
	Effects::SkinningFX->ForwardTexNormalMapTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::PosNormalTexTangentUWeightsBoneIndices, 6, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &PosNormalTexTangentUWeightsBoneIndices));

	// Particle
	Effects::FireFX->StreamOutTech->GetPassByIndex(0)->GetDesc(&passDesc);
	HR(device->CreateInputLayout(InputLayoutDesc::Particle, 5, passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize, &Particle));
}

void InputLayouts::DestroyAll()
{
	ReleaseCOM(PosColor);
	ReleaseCOM(PosNormalTex);
	ReleaseCOM(InstancedPosNormalTex);
	ReleaseCOM(PosNormalTexTangentU);
	ReleaseCOM(PosNormalTexTangentUWeightsBoneIndices);
	ReleaseCOM(Particle);
}

#pragma endregion