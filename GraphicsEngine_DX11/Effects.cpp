//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"
#include "MacroDefine.h"
#include "ErrChecker.h"

#include <fstream>

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: m_FX(0), WorldViewProj()
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();

	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size,
		0, device, &m_FX));
}

Effect::~Effect()
{
}
#pragma endregion

#pragma region ColorEffect
ColorEffect::ColorEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

ColorEffect::~ColorEffect()
{
}

void ColorEffect::GetTech()
{
	ColorTech = m_FX->GetTechniqueByName("ColorTech");
	Color_NoDepthTech = m_FX->GetTechniqueByName("Color_NoDepthTech");
}

#pragma endregion

#pragma region SkyEffect
SkyEffect::SkyEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	View = m_FX->GetVariableByName("gView")->AsMatrix();
	Proj = m_FX->GetVariableByName("gProj")->AsMatrix();
	CubeMap = m_FX->GetVariableByName("gCubeMap")->AsShaderResource();
	Texture = m_FX->GetVariableByName("gTexture")->AsShaderResource();
	Roughness = m_FX->GetVariableByName("gRoughness")->AsScalar();
}

SkyEffect::~SkyEffect()
{
}

void SkyEffect::GetTech()
{
	SkyTech = m_FX->GetTechniqueByName("SkyTech");
	DeferredSkyTech = m_FX->GetTechniqueByName("DeferredSky");
	RectToCubeMapTech = m_FX->GetTechniqueByName("RectToCubeMapTech");
	IrradianceTech = m_FX->GetTechniqueByName("IrradianceTech");
	PreFilterTech = m_FX->GetTechniqueByName("PreFilterTech");
	IntegrateBRDFTech = m_FX->GetTechniqueByName("IntegrateBRDFTech");
}

#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();

	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = m_FX->GetVariableByName("gWorld")->AsMatrix();
	WorldView = m_FX->GetVariableByName("gWorldView")->AsMatrix();
	WorldInvTranspose = m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	WorldInvTransposeView = m_FX->GetVariableByName("gWorldInvTransposeView")->AsMatrix();
	ViewProj = m_FX->GetVariableByName("gViewProj")->AsMatrix();

	TexTransform = m_FX->GetVariableByName("gTexTransform")->AsMatrix();
	ShadowTransform = m_FX->GetVariableByName("gShadowTransform")->AsMatrix();

	EyePosW = m_FX->GetVariableByName("gEyePosW")->AsVector();
	DirLights = m_FX->GetVariableByName("gDirLights");
	PLight = m_FX->GetVariableByName("gPointLights");
	SLight = m_FX->GetVariableByName("gSpotLight");
	DirLightCount = m_FX->GetVariableByName("gDirLightCount")->AsScalar();
	PLightCount = m_FX->GetVariableByName("gPointLightCount")->AsScalar();
	SLightCount = m_FX->GetVariableByName("gSpotLightCount")->AsScalar();
	Alpha = m_FX->GetVariableByName("gAlpha")->AsScalar();
	Mat = m_FX->GetVariableByName("gMaterial");

	Metallic = m_FX->GetVariableByName("gMetallic")->AsScalar();
	Roughness = m_FX->GetVariableByName("gRoughness")->AsScalar();

	DiffuseMap = m_FX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	CubeMap = m_FX->GetVariableByName("gCubeMap")->AsShaderResource();
	ShadowMap = m_FX->GetVariableByName("gShadowMap")->AsShaderResource();

	MetallicMap = m_FX->GetVariableByName("gMetallicMap")->AsShaderResource();
	RoughnessMap = m_FX->GetVariableByName("gRoughnessMap")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
}

void BasicEffect::SetDirectionalLightDataVec(const std::vector<DirectionalLightInfo*> lights)
{
	// 쉐이더에 보낼 Light Data 벡터 생성
	std::vector<DirectionalLightData> dirLightData(lights.size());

	for (UINT i = 0; i < lights.size(); i++)
	{
		dirLightData[i].Ambient.x = lights[i]->Ambient.x;
		dirLightData[i].Ambient.y = lights[i]->Ambient.y;
		dirLightData[i].Ambient.z = lights[i]->Ambient.z;
		dirLightData[i].Ambient.w = lights[i]->Ambient.w;

		dirLightData[i].Diffuse.x = lights[i]->Diffuse.x;
		dirLightData[i].Diffuse.y = lights[i]->Diffuse.y;
		dirLightData[i].Diffuse.z = lights[i]->Diffuse.z;
		dirLightData[i].Diffuse.w = lights[i]->Diffuse.w;

		dirLightData[i].Specular.x = lights[i]->Specular.x;
		dirLightData[i].Specular.y = lights[i]->Specular.y;
		dirLightData[i].Specular.z = lights[i]->Specular.z;
		dirLightData[i].Specular.w = lights[i]->Specular.w;

		dirLightData[i].Direction.x = lights[i]->Direction.x;
		dirLightData[i].Direction.y = lights[i]->Direction.y;
		dirLightData[i].Direction.z = lights[i]->Direction.z;
	}

	if (lights.size() > 0)
	{
		DirLights->SetRawValue(&dirLightData[0], 0, (uint32_t)lights.size() * sizeof(DirectionalLightData));

		// 라이트의 총 개수도 알려줌
		DirLightCount->SetInt((int)lights.size());
	}
}

void BasicEffect::SetPointLightDataVec(const std::vector<PointLightInfo*> lights)
{
	// 쉐이더에 보낼 Light Data 벡터 생성
	std::vector<PointLightData> pointLightData(lights.size());

	for (UINT i = 0; i < lights.size(); i++)
	{
		pointLightData[i].Ambient.x = lights[i]->Ambient.x;
		pointLightData[i].Ambient.y = lights[i]->Ambient.y;
		pointLightData[i].Ambient.z = lights[i]->Ambient.z;
		pointLightData[i].Ambient.w = lights[i]->Ambient.w;

		pointLightData[i].Diffuse.x = lights[i]->Diffuse.x;
		pointLightData[i].Diffuse.y = lights[i]->Diffuse.y;
		pointLightData[i].Diffuse.z = lights[i]->Diffuse.z;
		pointLightData[i].Diffuse.w = lights[i]->Diffuse.w;

		pointLightData[i].Specular.x = lights[i]->Specular.x;
		pointLightData[i].Specular.y = lights[i]->Specular.y;
		pointLightData[i].Specular.z = lights[i]->Specular.z;
		pointLightData[i].Specular.w = lights[i]->Specular.w;

		pointLightData[i].Position.x = lights[i]->Position.x;
		pointLightData[i].Position.y = lights[i]->Position.y;
		pointLightData[i].Position.z = lights[i]->Position.z;

		pointLightData[i].Range = lights[i]->Range;

		pointLightData[i].Att.x = lights[i]->Att.x;
		pointLightData[i].Att.y = lights[i]->Att.y;
		pointLightData[i].Att.z = lights[i]->Att.z;
	}

	if (lights.size() > 0)
	{
		PLight->SetRawValue(&pointLightData[0], 0, (uint32_t)lights.size() * sizeof(PointLightData));

		// 라이트의 총 개수도 알려줌
		PLightCount->SetInt((int)lights.size());
	}
}

void BasicEffect::SetSpotLightDataVec(const std::vector<SpotLightInfo*> lights)
{
	// 쉐이더에 보낼 Light Data 벡터 생성
	std::vector<SpotLightData> spotLightData(lights.size());

	for (UINT i = 0; i < lights.size(); i++)
	{
		spotLightData[i].Ambient.x = lights[i]->Ambient.x;
		spotLightData[i].Ambient.y = lights[i]->Ambient.y;
		spotLightData[i].Ambient.z = lights[i]->Ambient.z;
		spotLightData[i].Ambient.w = lights[i]->Ambient.w;

		spotLightData[i].Diffuse.x = lights[i]->Diffuse.x;
		spotLightData[i].Diffuse.y = lights[i]->Diffuse.y;
		spotLightData[i].Diffuse.z = lights[i]->Diffuse.z;
		spotLightData[i].Diffuse.w = lights[i]->Diffuse.w;

		spotLightData[i].Specular.x = lights[i]->Specular.x;
		spotLightData[i].Specular.y = lights[i]->Specular.y;
		spotLightData[i].Specular.z = lights[i]->Specular.z;
		spotLightData[i].Specular.w = lights[i]->Specular.w;

		spotLightData[i].Position.x = lights[i]->Position.x;
		spotLightData[i].Position.y = lights[i]->Position.y;
		spotLightData[i].Position.z = lights[i]->Position.z;

		spotLightData[i].Range = lights[i]->Range;

		spotLightData[i].Direction.x = lights[i]->Direction.x;
		spotLightData[i].Direction.y = lights[i]->Direction.y;
		spotLightData[i].Direction.z = lights[i]->Direction.z;

		spotLightData[i].Spot = lights[i]->Spot;

		spotLightData[i].Att.x = lights[i]->Att.x;
		spotLightData[i].Att.y = lights[i]->Att.y;
		spotLightData[i].Att.z = lights[i]->Att.z;
	}

	if (lights.size() > 0)
	{
		SLight->SetRawValue(&spotLightData[0], 0, (uint32_t)lights.size() * sizeof(SpotLightData));

		// 라이트의 총 개수도 알려줌
		SLightCount->SetInt((int)lights.size());
	}
}

void BasicEffect::GetTech()
{
	ForwardTech = m_FX->GetTechniqueByName("Forward");
	ForwardTexTech = m_FX->GetTechniqueByName("ForwardTex");

	Forward_InstancingTech = m_FX->GetTechniqueByName("Forward_Instancing");
	ForwardTex_InstancingTech = m_FX->GetTechniqueByName("ForwardTex_Instancing");

	DeferredTech = m_FX->GetTechniqueByName("Deferred");
	DeferredTexTech = m_FX->GetTechniqueByName("DeferredTex");
}

#pragma endregion

#pragma region NormalMapEffect
NormalMapEffect::NormalMapEffect(ID3D11Device* device, const std::wstring& filename)
	: BasicEffect(device, filename)
{
	GetTech();
	NormalMap = m_FX->GetVariableByName("gNormalMap")->AsShaderResource();
}

NormalMapEffect::~NormalMapEffect()
{
}

void NormalMapEffect::GetTech()
{
	ForwardTexNormalMapTech = m_FX->GetTechniqueByName("ForwardTexNormalMap");
	DeferredTexNormalMapTech = m_FX->GetTechniqueByName("DeferredTexNormalMap");
}

#pragma endregion

#pragma region SkinningEffect
SkinningEffect::SkinningEffect(ID3D11Device* device, const std::wstring& filename)
	: NormalMapEffect(device, filename)
{
	GetTech();
	BoneTransforms = m_FX->GetVariableByName("gBoneTransforms")->AsMatrix();
}

SkinningEffect::~SkinningEffect()
{
}

void SkinningEffect::SetBoneTransforms(const std::vector<EMath::FLOAT4X4> boneTransformVec)
{
	BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(&boneTransformVec[0].m), 0, (uint32_t)boneTransformVec.size());
}

void SkinningEffect::GetTech()
{
	ForwardTech = m_FX->GetTechniqueByName("Forward");
	ForwardTexTech = m_FX->GetTechniqueByName("ForwardTex");
	ForwardTexNormalMapTech = m_FX->GetTechniqueByName("ForwardTexNormalMap");

	DeferredTech = m_FX->GetTechniqueByName("Deferred");
	DeferredTexTech = m_FX->GetTechniqueByName("DeferredTex");
	DeferredTexNormalMapTech = m_FX->GetTechniqueByName("DeferredTexNormalMap");
}

#pragma endregion

#pragma region BlurEffect
BlurEffect::BlurEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	Weights = m_FX->GetVariableByName("gWeights")->AsScalar();
	InputMap = m_FX->GetVariableByName("gInput")->AsShaderResource();
	OutputMap = m_FX->GetVariableByName("gOutput")->AsUnorderedAccessView();
}

BlurEffect::~BlurEffect()
{

}

void BlurEffect::GetTech()
{
	HorzBlurTech = m_FX->GetTechniqueByName("HorzBlur");
	VertBlurTech = m_FX->GetTechniqueByName("VertBlur");
}

#pragma endregion

#pragma region DOFEffect
DOFEffect::DOFEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();

	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();

	Screen = m_FX->GetVariableByName("SrcTex")->AsShaderResource();
	BlurScreen = m_FX->GetVariableByName("DOFBlurTex")->AsShaderResource();
	DepthMap = m_FX->GetVariableByName("DepthTex")->AsShaderResource();

	ProjValues = m_FX->GetVariableByName("ProjValues")->AsVector();
	DOFFarValues = m_FX->GetVariableByName("DOFFarValues")->AsVector();
}

DOFEffect::~DOFEffect()
{

}

void DOFEffect::GetTech()
{
	DOFTech = m_FX->GetTechniqueByName("DOFTech");
}

#pragma endregion

#pragma region ShadowMapEffect
ShadowMapEffect::ShadowMapEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	TexTransform = m_FX->GetVariableByName("gTexTransform")->AsMatrix();
}

ShadowMapEffect::~ShadowMapEffect()
{

}

void ShadowMapEffect::GetTech()
{
	BuildShadowMap_BasicTech = m_FX->GetTechniqueByName("BuildShadowMap_Basic");
	BuildShadowMap_NormalMapTech = m_FX->GetTechniqueByName("BuildShadowMap_NormalMap");
	BuildShadowMap_SkinningTech = m_FX->GetTechniqueByName("BuildShadowMap_Skinning");

	BuildShadowMapAlphaClipTech = m_FX->GetTechniqueByName("BuildShadowMapAlphaClip");
}

#pragma endregion

#pragma region DebugTexEffect
DebugTexEffect::DebugTexEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	Texture = m_FX->GetVariableByName("gTexture")->AsShaderResource();
}

DebugTexEffect::~DebugTexEffect()
{
}

void DebugTexEffect::GetTech()
{
	ViewArgbTech = m_FX->GetTechniqueByName("ViewArgbTech");
	ViewRedTech = m_FX->GetTechniqueByName("ViewRedTech");
	ViewGreenTech = m_FX->GetTechniqueByName("ViewGreenTech");
	ViewBlueTech = m_FX->GetTechniqueByName("ViewBlueTech");
	ViewAlphaTech = m_FX->GetTechniqueByName("ViewAlphaTech");
}

#pragma endregion

#pragma region DeferredLightingEffect
DeferredLightingEffect::DeferredLightingEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();

	Albedo = m_FX->GetVariableByName("gAlbedo")->AsShaderResource();
	MaterialRT = m_FX->GetVariableByName("gMaterialRT")->AsShaderResource();
	PosW = m_FX->GetVariableByName("gPosW")->AsShaderResource();
	PosH = m_FX->GetVariableByName("gPosH")->AsShaderResource();
	NormalW = m_FX->GetVariableByName("gNormalW")->AsShaderResource();
	ShadowPosH = m_FX->GetVariableByName("gShadowPosH")->AsShaderResource();
	ShadowMap = m_FX->GetVariableByName("gShadowMap")->AsShaderResource();
	SSAOMap = m_FX->GetVariableByName("gSSAOMap")->AsShaderResource();

	// IBL Map
	IrradianceMap = m_FX->GetVariableByName("gIrradianceMap")->AsShaderResource();
	PreFilterMap = m_FX->GetVariableByName("gPreFilterMap")->AsShaderResource();
	BrdfLUT = m_FX->GetVariableByName("gBrdfLUT")->AsShaderResource();

	RenderTargetVec.push_back(Albedo);
	RenderTargetVec.push_back(MaterialRT);
	RenderTargetVec.push_back(PosW);
	RenderTargetVec.push_back(PosH);
	RenderTargetVec.push_back(NormalW);
	RenderTargetVec.push_back(ShadowPosH);

	// Lights
	EyePosW = m_FX->GetVariableByName("gEyePosW")->AsVector();
	DirLights = m_FX->GetVariableByName("gDirLights");
	PLight = m_FX->GetVariableByName("gPointLights");
	SLight = m_FX->GetVariableByName("gSpotLight");
	DirLightCount = m_FX->GetVariableByName("gDirLightCount")->AsScalar();
	PLightCount = m_FX->GetVariableByName("gPointLightCount")->AsScalar();
	SLightCount = m_FX->GetVariableByName("gSpotLightCount")->AsScalar();

	ViewProjTex = m_FX->GetVariableByName("gViewProjTex")->AsMatrix();
}

DeferredLightingEffect::~DeferredLightingEffect()
{

}

void DeferredLightingEffect::SetRenderTargets(std::vector<ID3D11ShaderResourceView*> renderTargets)
{
	// sort 되어져서 이 곳에 오므로 순서대로 넣어주면 된다.
	for (size_t i = 0; i < renderTargets.size(); i++)
	{
		RenderTargetVec[i]->SetResource(renderTargets[i]);
	}
}

void DeferredLightingEffect::SetDirectionalLightDataVec(const std::vector<DirectionalLightInfo*> lights)
{
	// 쉐이더에 보낼 Light Data 벡터 생성
	std::vector<DirectionalLightData> dirLightData(lights.size());

	for (UINT i = 0; i < lights.size(); i++)
	{
		dirLightData[i].Ambient.x = lights[i]->Ambient.x;
		dirLightData[i].Ambient.y = lights[i]->Ambient.y;
		dirLightData[i].Ambient.z = lights[i]->Ambient.z;
		dirLightData[i].Ambient.w = lights[i]->Ambient.w;

		dirLightData[i].Diffuse.x = lights[i]->Diffuse.x;
		dirLightData[i].Diffuse.y = lights[i]->Diffuse.y;
		dirLightData[i].Diffuse.z = lights[i]->Diffuse.z;
		dirLightData[i].Diffuse.w = lights[i]->Diffuse.w;

		dirLightData[i].Specular.x = lights[i]->Specular.x;
		dirLightData[i].Specular.y = lights[i]->Specular.y;
		dirLightData[i].Specular.z = lights[i]->Specular.z;
		dirLightData[i].Specular.w = lights[i]->Specular.w;

		dirLightData[i].Direction.x = lights[i]->Direction.x;
		dirLightData[i].Direction.y = lights[i]->Direction.y;
		dirLightData[i].Direction.z = lights[i]->Direction.z;
	}

	if (lights.size() > 0)
	{
		DirLights->SetRawValue(&dirLightData[0], 0, (uint32_t)lights.size() * sizeof(DirectionalLightData));

		// 라이트의 총 개수도 알려줌
		DirLightCount->SetInt((int)lights.size());
	}
}

void DeferredLightingEffect::SetPointLightDataVec(const std::vector<PointLightInfo*> lights)
{
	// 쉐이더에 보낼 Light Data 벡터 생성
	std::vector<PointLightData> pointLightData(lights.size());

	for (UINT i = 0; i < lights.size(); i++)
	{
		pointLightData[i].Ambient.x = lights[i]->Ambient.x;
		pointLightData[i].Ambient.y = lights[i]->Ambient.y;
		pointLightData[i].Ambient.z = lights[i]->Ambient.z;
		pointLightData[i].Ambient.w = lights[i]->Ambient.w;

		pointLightData[i].Diffuse.x = lights[i]->Diffuse.x;
		pointLightData[i].Diffuse.y = lights[i]->Diffuse.y;
		pointLightData[i].Diffuse.z = lights[i]->Diffuse.z;
		pointLightData[i].Diffuse.w = lights[i]->Diffuse.w;

		pointLightData[i].Specular.x = lights[i]->Specular.x;
		pointLightData[i].Specular.y = lights[i]->Specular.y;
		pointLightData[i].Specular.z = lights[i]->Specular.z;
		pointLightData[i].Specular.w = lights[i]->Specular.w;

		pointLightData[i].Position.x = lights[i]->Position.x;
		pointLightData[i].Position.y = lights[i]->Position.y;
		pointLightData[i].Position.z = lights[i]->Position.z;

		pointLightData[i].Range = lights[i]->Range;

		pointLightData[i].Att.x = lights[i]->Att.x;
		pointLightData[i].Att.y = lights[i]->Att.y;
		pointLightData[i].Att.z = lights[i]->Att.z;
	}

	if (lights.size() > 0)
	{
		PLight->SetRawValue(&pointLightData[0], 0, (uint32_t)lights.size() * sizeof(PointLightData));
		
		// 라이트의 총 개수도 알려줌
		PLightCount->SetInt((int)lights.size());
	}
}

void DeferredLightingEffect::SetSpotLightDataVec(const std::vector<SpotLightInfo*> lights)
{
	// 쉐이더에 보낼 Light Data 벡터 생성
	std::vector<SpotLightData> spotLightData(lights.size());

	for (UINT i = 0; i < lights.size(); i++)
	{
		spotLightData[i].Ambient.x = lights[i]->Ambient.x;
		spotLightData[i].Ambient.y = lights[i]->Ambient.y;
		spotLightData[i].Ambient.z = lights[i]->Ambient.z;
		spotLightData[i].Ambient.w = lights[i]->Ambient.w;

		spotLightData[i].Diffuse.x = lights[i]->Diffuse.x;
		spotLightData[i].Diffuse.y = lights[i]->Diffuse.y;
		spotLightData[i].Diffuse.z = lights[i]->Diffuse.z;
		spotLightData[i].Diffuse.w = lights[i]->Diffuse.w;

		spotLightData[i].Specular.x = lights[i]->Specular.x;
		spotLightData[i].Specular.y = lights[i]->Specular.y;
		spotLightData[i].Specular.z = lights[i]->Specular.z;
		spotLightData[i].Specular.w = lights[i]->Specular.w;

		spotLightData[i].Position.x = lights[i]->Position.x;
		spotLightData[i].Position.y = lights[i]->Position.y;
		spotLightData[i].Position.z = lights[i]->Position.z;

		spotLightData[i].Range = lights[i]->Range;

		spotLightData[i].Direction.x = lights[i]->Direction.x;
		spotLightData[i].Direction.y = lights[i]->Direction.y;
		spotLightData[i].Direction.z = lights[i]->Direction.z;

		spotLightData[i].Spot = lights[i]->Spot;

		spotLightData[i].Att.x = lights[i]->Att.x;
		spotLightData[i].Att.y = lights[i]->Att.y;
		spotLightData[i].Att.z = lights[i]->Att.z;
	}

	if (lights.size() > 0)
	{
		SLight->SetRawValue(&spotLightData[0], 0, (uint32_t)lights.size() * sizeof(SpotLightData));

		// 라이트의 총 개수도 알려줌
		SLightCount->SetInt((int)lights.size());
	}
}

void DeferredLightingEffect::GetTech()
{
	LightingTech = m_FX->GetTechniqueByName("Lighting");
	LightingShadowTech = m_FX->GetTechniqueByName("LightingShadow");
	LightingSSAOTech = m_FX->GetTechniqueByName("LightingSSAO");
	LightingShadowSSAOTech = m_FX->GetTechniqueByName("LightingShadowSSAO");
}
#pragma endregion

#pragma region SSAOEffect
SSAOEffect::SSAOEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();

	View = m_FX->GetVariableByName("gView")->AsMatrix();
	ViewToTexSpace = m_FX->GetVariableByName("gViewToTexSpace")->AsMatrix();
	OffsetVectors = m_FX->GetVariableByName("gOffsetVectors")->AsVector();
	FrustumCorners = m_FX->GetVariableByName("gFrustumCorners")->AsVector();
	OcclusionRadius = m_FX->GetVariableByName("gOcclusionRadius")->AsScalar();
	OcclusionFadeStart = m_FX->GetVariableByName("gOcclusionFadeStart")->AsScalar();
	OcclusionFadeEnd = m_FX->GetVariableByName("gOcclusionFadeEnd")->AsScalar();
	SurfaceEpsilon = m_FX->GetVariableByName("gSurfaceEpsilon")->AsScalar();

	NormalDepthRT = m_FX->GetVariableByName("gNormalDepthRT")->AsShaderResource();
	DepthRT = m_FX->GetVariableByName("gDepthRT")->AsShaderResource();
	RandomVecMap = m_FX->GetVariableByName("gRandomVecMap")->AsShaderResource();
}

SSAOEffect::~SSAOEffect()
{
}

void SSAOEffect::GetTech()
{
	SSAOTech = m_FX->GetTechniqueByName("Ssao");
}

#pragma endregion

#pragma region SSAOBlurEffect
SSAOBlurEffect::SSAOBlurEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();

	TexelWidth = m_FX->GetVariableByName("gTexelWidth")->AsScalar();
	TexelHeight = m_FX->GetVariableByName("gTexelHeight")->AsScalar();

	NormalDepthRT = m_FX->GetVariableByName("gNormalDepthRT")->AsShaderResource();
	DepthRT = m_FX->GetVariableByName("gDepthRT")->AsShaderResource();
	InputImage = m_FX->GetVariableByName("gInputImage")->AsShaderResource();
}

SSAOBlurEffect::~SSAOBlurEffect()
{
}

void SSAOBlurEffect::GetTech()
{
	HorzBlurTech = m_FX->GetTechniqueByName("HorzBlur");
	VertBlurTech = m_FX->GetTechniqueByName("VertBlur");
}

#pragma endregion

#pragma region MixTextureEffect
MixTextureEffect::MixTextureEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();

	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	Texture_1 = m_FX->GetVariableByName("gTexture_1")->AsShaderResource();
	Texture_2 = m_FX->GetVariableByName("gTexture_2")->AsShaderResource();
	Texture_3 = m_FX->GetVariableByName("gTexture_3")->AsShaderResource();

	Per = m_FX->GetVariableByName("gPer")->AsScalar();
	FadePer = m_FX->GetVariableByName("gFadePer")->AsScalar();
}

MixTextureEffect::~MixTextureEffect()
{

}

void MixTextureEffect::GetTech()
{
	MixTextureTech = m_FX->GetTechniqueByName("MixTexture");
	MixTexture_AccumulateTech = m_FX->GetTechniqueByName("MixTexture_Accumulate");
	MixTexture_MagicEyeTech = m_FX->GetTechniqueByName("MixTexture_MagicEye");
	MixTexture_NotMagicEyeTech = m_FX->GetTechniqueByName("MixTexture_NotMagicEye");
	MixTexture_FadeInOutTech = m_FX->GetTechniqueByName("MixTexture_FadeInOut");
	MixTexture_MixBloomTech = m_FX->GetTechniqueByName("MixTexture_MixBloom");
}
#pragma endregion

#pragma region FXAAEffect
FXAAEffect::FXAAEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	Texture = m_FX->GetVariableByName("InputTexture")->AsShaderResource();
	RCPFrame = m_FX->GetVariableByName("RCPFrame")->AsVector();
}

FXAAEffect::~FXAAEffect()
{

}

void FXAAEffect::GetTech()
{
	FXAATech = m_FX->GetTechniqueByName("FXAA");
}
#pragma endregion

#pragma region OutlineEffect
OutlineEffect::OutlineEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	WorldInvTranspose = m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	NormalW = m_FX->GetVariableByName("gNormalW")->AsShaderResource();
	Final = m_FX->GetVariableByName("gFinal")->AsShaderResource();
	Outline = m_FX->GetVariableByName("gOutline")->AsShaderResource();
	DepthMap = m_FX->GetVariableByName("gDepthMap")->AsShaderResource();
	BoneTransforms = m_FX->GetVariableByName("gBoneTransforms")->AsMatrix();
	Size = m_FX->GetVariableByName("gSize")->AsScalar();
}

OutlineEffect::~OutlineEffect()
{

}

void OutlineEffect::GetTech()
{
	Basic_DepthTech = m_FX->GetTechniqueByName("Basic_Depth");
	NormalMap_DepthTech = m_FX->GetTechniqueByName("NormalMap_Depth");
	Skinning_DepthTech = m_FX->GetTechniqueByName("Skinning_Depth");

	Basic_NormalTech = m_FX->GetTechniqueByName("Basic_Normal");
	NormalMap_NormalTech = m_FX->GetTechniqueByName("NormalMap_Normal");
	Skinning_NormalTech = m_FX->GetTechniqueByName("Skinning_Normal");

	Basic_OverSizeTech = m_FX->GetTechniqueByName("Basic_OverSize");
	NormalMap_OverSizeTech = m_FX->GetTechniqueByName("NormalMap_OverSize");
	Skinning_OverSizeTech = m_FX->GetTechniqueByName("Skinning_OverSize");

	Outline_LaplacianTech = m_FX->GetTechniqueByName("Outline_Laplacian");
	Mix_LaplacianTech = m_FX->GetTechniqueByName("Mix_Laplacian");
	Mix_OverSizeTech = m_FX->GetTechniqueByName("Mix_OverSize");
}

void OutlineEffect::SetBoneTransforms(const std::vector<EMath::FLOAT4X4> boneTransformVec)
{
	BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(&boneTransformVec[0].m), 0, (uint32_t)boneTransformVec.size());
}
#pragma endregion

#pragma region MotionBlurEffect
MotionBlurEffect::MotionBlurEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	ViewProjInv = m_FX->GetVariableByName("gViewProjInv")->AsMatrix();
	ViewProj_Prev = m_FX->GetVariableByName("gViewProj_Prev")->AsMatrix();

	DepthMap = m_FX->GetVariableByName("gDepthMap")->AsShaderResource();
	Final = m_FX->GetVariableByName("gFinal")->AsShaderResource();
}

MotionBlurEffect::~MotionBlurEffect()
{

}

void MotionBlurEffect::GetTech()
{
	MotionBlurTech = m_FX->GetTechniqueByName("MotionBlur");
}
#pragma endregion

#pragma region ToneMappingEffect
ToneMappingEffect::ToneMappingEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	Input = m_FX->GetVariableByName("gInput")->AsShaderResource();
}

ToneMappingEffect::~ToneMappingEffect()
{

}

void ToneMappingEffect::GetTech()
{
	ToneMapping_Simple_Tech = m_FX->GetTechniqueByName("ToneMapping_Simple");
	ToneMapping_Simple_Invert_Tech = m_FX->GetTechniqueByName("ToneMapping_Simple_Invert");

	ToneMapping_ACES_Tech = m_FX->GetTechniqueByName("ToneMapping_ACES");
	ToneMapping_Reinhard_Tech = m_FX->GetTechniqueByName("ToneMapping_Reinhard");
}
#pragma endregion

#pragma region EmissiveEffect
EmissiveEffect::EmissiveEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
	BoneTransforms = m_FX->GetVariableByName("gBoneTransforms")->AsMatrix();

	EmissiveMap = m_FX->GetVariableByName("gEmissiveMap")->AsShaderResource();
	Final = m_FX->GetVariableByName("gFinal")->AsShaderResource();
	EmissiveBluredOutput = m_FX->GetVariableByName("gEmissiveBluredOutput")->AsShaderResource();
}

EmissiveEffect::~EmissiveEffect()
{

}

void EmissiveEffect::SetBoneTransforms(const std::vector<EMath::FLOAT4X4> boneTransformVec)
{
	BoneTransforms->SetMatrixArray(reinterpret_cast<const float*>(&boneTransformVec[0].m), 0, (uint32_t)boneTransformVec.size());
}

void EmissiveEffect::GetTech()
{
	BasicTech = m_FX->GetTechniqueByName("Basic");
	NormalMapTech = m_FX->GetTechniqueByName("NormalMap");
	SkinningTech = m_FX->GetTechniqueByName("Skinning");
	MixTech = m_FX->GetTechniqueByName("Mix");
}
#pragma endregion

#pragma region EmissiveEffect
BloomEffect::BloomEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();
	WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();

	Texture = m_FX->GetVariableByName("gTexture")->AsShaderResource();
	Threshold = m_FX->GetVariableByName("gThreshold")->AsScalar();
}

BloomEffect::~BloomEffect()
{

}

void BloomEffect::GetTech()
{
	BloomCurveTech = m_FX->GetTechniqueByName("BloomCurve");
}
#pragma endregion

#pragma region ParticleEffect
ParticleEffect::ParticleEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	GetTech();

	ViewProj = m_FX->GetVariableByName("gViewProj")->AsMatrix();
	GameTime = m_FX->GetVariableByName("gGameTime")->AsScalar();
	TimeStep = m_FX->GetVariableByName("gTimeStep")->AsScalar();
	EyePosW = m_FX->GetVariableByName("gEyePosW")->AsVector();
	EmitPosW = m_FX->GetVariableByName("gEmitPosW")->AsVector();
	EmitDirW = m_FX->GetVariableByName("gEmitDirW")->AsVector();
	TexArray = m_FX->GetVariableByName("gTexArray")->AsShaderResource();
	RandomTex = m_FX->GetVariableByName("gRandomTex")->AsShaderResource();
}

ParticleEffect::~ParticleEffect()
{
}

void ParticleEffect::GetTech()
{
	StreamOutTech = m_FX->GetTechniqueByName("StreamOutTech");
	DrawTech = m_FX->GetTechniqueByName("DrawTech");
}
#pragma endregion

#pragma region Effects
ColorEffect* Effects::ColorFX = nullptr;
BasicEffect* Effects::BasicFX = nullptr;
NormalMapEffect* Effects::NormalMapFX = nullptr;
SkinningEffect* Effects::SkinningFX = nullptr;
SkyEffect* Effects::SkyFX = nullptr;
BlurEffect* Effects::BlurFX = nullptr;
DOFEffect* Effects::DofFX = nullptr;
ShadowMapEffect* Effects::ShadowMapFX = nullptr;
DebugTexEffect* Effects::DebugTexFX = nullptr;
DeferredLightingEffect* Effects::DeferredLightingFX = nullptr;
DeferredLightingEffect* Effects::DeferredLightingPBRFX = nullptr;
SSAOEffect* Effects::SSAOFX = nullptr;
SSAOBlurEffect* Effects::SSAOBlurFX = nullptr;
MixTextureEffect* Effects::MixTextureFX = nullptr;
FXAAEffect* Effects::FXAAFX = nullptr;
OutlineEffect* Effects::OutlineFX = nullptr;
MotionBlurEffect* Effects::MotionBlurFX = nullptr;
ToneMappingEffect* Effects::ToneMappingFX = nullptr;
EmissiveEffect* Effects::EmissiveFX = nullptr;
BloomEffect* Effects::BloomFX = nullptr;
ParticleEffect* Effects::FireFX = nullptr;

std::vector<BasicEffect*> Effects::BasicEffectVec;

void Effects::InitAll(ID3D11Device* device)
{
	std::wstring folderPath = L"../../Data/FX/";

	ColorFX = new ColorEffect(device, folderPath + L"Color.fxo");

	BasicFX = new BasicEffect(device, folderPath + L"Basic.fxo");
	NormalMapFX = new NormalMapEffect(device, folderPath + L"NormalMap.fxo");
	SkinningFX = new SkinningEffect(device, folderPath + L"Skinning.fxo");

	SkyFX = new SkyEffect(device, folderPath + L"Sky.fxo");

	BlurFX = new BlurEffect(device, folderPath + L"Blur.fxo");
	DofFX = new DOFEffect(device, folderPath + L"DepthOfField.fxo");

	ShadowMapFX = new ShadowMapEffect(device, folderPath + L"ShadowMap.fxo");
	DebugTexFX = new DebugTexEffect(device, folderPath + L"DebugTexture.fxo");

	DeferredLightingFX = new DeferredLightingEffect(device, folderPath + L"DeferredLighting.fxo");
	DeferredLightingPBRFX = new DeferredLightingEffect(device, folderPath + L"DeferredLighting_PBR.fxo");

	SSAOFX = new SSAOEffect(device, folderPath + L"SSAO.fxo");
	SSAOBlurFX = new SSAOBlurEffect(device, folderPath + L"SSAOBlur.fxo");

	MixTextureFX = new MixTextureEffect(device, folderPath + L"MixTexture.fxo");
	FXAAFX = new FXAAEffect(device, folderPath + L"FXAA.fxo");

	OutlineFX = new OutlineEffect(device, folderPath + L"Outline.fxo");
	MotionBlurFX = new MotionBlurEffect(device, folderPath + L"MotionBlur.fxo");
	ToneMappingFX = new ToneMappingEffect(device, folderPath + L"ToneMapping.fxo");
	EmissiveFX = new EmissiveEffect(device, folderPath + L"Emissive.fxo");
	BloomFX = new BloomEffect(device, folderPath + L"Bloom.fxo");

	FireFX = new ParticleEffect(device, folderPath + L"Fire.fxo");

	BasicEffectVec.push_back(BasicFX);
	BasicEffectVec.push_back(NormalMapFX);
	BasicEffectVec.push_back(SkinningFX);
}

void Effects::DestroyAll()
{
	SafeDelete(FireFX);
	SafeDelete(BloomFX);
	SafeDelete(EmissiveFX);
	SafeDelete(ToneMappingFX);
	SafeDelete(MotionBlurFX);
	SafeDelete(OutlineFX);
	SafeDelete(FXAAFX);
	SafeDelete(MixTextureFX);
	SafeDelete(SSAOFX);
	SafeDelete(SSAOBlurFX);
	SafeDelete(DeferredLightingPBRFX);
	SafeDelete(DeferredLightingFX);
	SafeDelete(DebugTexFX);
	SafeDelete(ShadowMapFX);
	SafeDelete(DofFX);
	SafeDelete(BlurFX);
	SafeDelete(SkyFX);
	SafeDelete(SkinningFX);
	SafeDelete(NormalMapFX);
	SafeDelete(BasicFX);
	SafeDelete(ColorFX);
}

void Effects::SetEyePos(const EMath::FLOAT3& v)
{
	DeferredLightingFX->SetEyePosW(v);
	DeferredLightingPBRFX->SetEyePosW(v);
	FireFX->SetEyePosW(v);

	for (const auto& it : BasicEffectVec)
	{
		it->SetEyePosW(v);
	}
}

void Effects::SetDirectionalLightVec(const std::vector<DirectionalLightInfo*> lights)
{
	for (const auto& it : BasicEffectVec)
	{
		it->SetDirectionalLightDataVec(lights);
	}

	DeferredLightingFX->SetDirectionalLightDataVec(lights);
	DeferredLightingPBRFX->SetDirectionalLightDataVec(lights);
}

void Effects::SetPointLightVec(const std::vector<PointLightInfo*> lights)
{
	for (const auto& it : BasicEffectVec)
	{
		it->SetPointLightDataVec(lights);
	}

	DeferredLightingFX->SetPointLightDataVec(lights);
	DeferredLightingPBRFX->SetPointLightDataVec(lights);
}

void Effects::SetSpotLightVec(const std::vector<SpotLightInfo*> lights)
{
	for (const auto& it : BasicEffectVec)
	{
		it->SetSpotLightDataVec(lights);
	}

	DeferredLightingFX->SetSpotLightDataVec(lights);
	DeferredLightingPBRFX->SetSpotLightDataVec(lights);
}

void Effects::SetShadowMap(ID3D11ShaderResourceView* shadowMap)
{
	for (const auto& it : BasicEffectVec)
	{
		it->SetShadowMap(shadowMap);
	}

	DeferredLightingFX->SetShadowMap(shadowMap);
	DeferredLightingPBRFX->SetShadowMap(shadowMap);
}

#pragma endregion
