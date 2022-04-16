//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include "LightDefine.h"

#include <d3dx11effect.h>
#include <vector>
#include <string>

#include <wrl.h>

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

private:
	Effect(const Effect& rhs);

protected:
	Microsoft::WRL::ComPtr<ID3DX11Effect> m_FX;
	ID3DX11EffectMatrixVariable* WorldViewProj;

public:
	void SetWorldViewProj(const EMath::FLOAT4X4& M) { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	
protected:
	virtual void GetTech() {};
};
#pragma endregion

#pragma region ColorEffect
class ColorEffect : public Effect
{
public:
	ColorEffect(ID3D11Device* device, const std::wstring& filename);
	~ColorEffect();

public:
	ID3DX11EffectTechnique* ColorTech;
	ID3DX11EffectTechnique* Color_NoDepthTech;

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region SkyEffect
class SkyEffect : public Effect
{
public:
	SkyEffect(ID3D11Device* device, const std::wstring& filename);
	~SkyEffect();

public:
	ID3DX11EffectTechnique* SkyTech;
	ID3DX11EffectTechnique* DeferredSkyTech;
	ID3DX11EffectTechnique* RectToCubeMapTech;
	ID3DX11EffectTechnique* IrradianceTech;
	ID3DX11EffectTechnique* PreFilterTech;
	ID3DX11EffectTechnique* IntegrateBRDFTech;

private:
	ID3DX11EffectShaderResourceVariable* CubeMap;
	ID3DX11EffectShaderResourceVariable* Texture;
	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* Proj;
	ID3DX11EffectScalarVariable* Roughness;

public:
	void SetCubeMap(ID3D11ShaderResourceView* cubemap) { CubeMap->SetResource(cubemap); }
	void SetTexture(ID3D11ShaderResourceView* tex) { Texture->SetResource(tex); }
	void SetView(const EMath::FLOAT4X4& M) { View->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetProj(const EMath::FLOAT4X4& M) { Proj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetRoughness(const float& rou) { Roughness->SetFloat(rou); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
	BasicEffect(ID3D11Device* device, const std::wstring& filename);
	~BasicEffect();

public:
	// Forward
	ID3DX11EffectTechnique* ForwardTech;
	ID3DX11EffectTechnique* ForwardTexTech;

	ID3DX11EffectTechnique* Forward_InstancingTech;
	ID3DX11EffectTechnique* ForwardTex_InstancingTech;

	// Deferred
	ID3DX11EffectTechnique* DeferredTech;
	ID3DX11EffectTechnique* DeferredTexTech;

protected:
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldView;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* WorldInvTransposeView;
	ID3DX11EffectMatrixVariable* ViewProj;

	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* ShadowTransform;

	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* PLight;
	ID3DX11EffectVariable* SLight;
	ID3DX11EffectScalarVariable* DirLightCount;
	ID3DX11EffectScalarVariable* PLightCount;
	ID3DX11EffectScalarVariable* SLightCount;

	ID3DX11EffectScalarVariable* Alpha;
	ID3DX11EffectVariable* Mat;

	// PBR
	ID3DX11EffectScalarVariable* Metallic;
	ID3DX11EffectScalarVariable* Roughness;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;
	ID3DX11EffectShaderResourceVariable* CubeMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;

	// PBR
	ID3DX11EffectShaderResourceVariable* MetallicMap;
	ID3DX11EffectShaderResourceVariable* RoughnessMap;

public:
	void SetWorld(const EMath::FLOAT4X4& M) { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldView(const EMath::FLOAT4X4& M) { WorldView->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(const EMath::FLOAT4X4& M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTransposeView(const EMath::FLOAT4X4& M) { WorldInvTransposeView->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetViewProj(const EMath::FLOAT4X4& M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetTexTransform(const EMath::FLOAT4X4& M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetShadowTransform(const EMath::FLOAT4X4& M) { ShadowTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetEyePosW(const EMath::FLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(EMath::FLOAT3)); }

	void SetDirectionalLightDataVec(const std::vector<DirectionalLightInfo*> lights);
	void SetPointLightDataVec(const std::vector<PointLightInfo*> lights);
	void SetSpotLightDataVec(const std::vector<SpotLightInfo*> lights);

	void SetAlpha(const float& alpha) { Alpha->SetFloat(alpha); }
	void SetMaterial(const LegacyMaterialData& mat) { Mat->SetRawValue(&mat, 0, sizeof(LegacyMaterialData)); }

	void SetMetallicFLOAT(const float& metallic) { Metallic->SetFloat(metallic); }
	void SetRoughnessFLOAT(const float& roughness) { Roughness->SetFloat(roughness); }

	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }
	void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }

	void SetMetallicMap(ID3D11ShaderResourceView* tex) { MetallicMap->SetResource(tex); }
	void SetRoughnessMap(ID3D11ShaderResourceView* tex) { RoughnessMap->SetResource(tex); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region NormalMapEffect
class NormalMapEffect : public BasicEffect
{
public:
	NormalMapEffect(ID3D11Device* device, const std::wstring& filename);
	~NormalMapEffect();

public:
	ID3DX11EffectTechnique* ForwardTexNormalMapTech;

	// Deferred
	ID3DX11EffectTechnique* DeferredTexNormalMapTech;

protected:
	ID3DX11EffectShaderResourceVariable* NormalMap;

public:
	void SetNormalMap(ID3D11ShaderResourceView* tex) { NormalMap->SetResource(tex); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region SkinningEffect
class SkinningEffect : public NormalMapEffect
{
public:
	SkinningEffect(ID3D11Device* device, const std::wstring& filename);
	~SkinningEffect();

protected:
	ID3DX11EffectMatrixVariable* BoneTransforms;

public:
	void SetBoneTransforms(const std::vector<EMath::FLOAT4X4> boneTransformVec);

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region BlurEffect
class BlurEffect : public Effect
{
public:
	BlurEffect(ID3D11Device* device, const std::wstring& filename);
	~BlurEffect();

public:
	ID3DX11EffectTechnique* HorzBlurTech;
	ID3DX11EffectTechnique* VertBlurTech;

protected:
	ID3DX11EffectScalarVariable* Weights;
	ID3DX11EffectShaderResourceVariable* InputMap;
	ID3DX11EffectUnorderedAccessViewVariable* OutputMap;

public:
	void SetWeights(const float weights[11]) { Weights->SetFloatArray(weights, 0, 11); }
	void SetInputMap(ID3D11ShaderResourceView* tex) { InputMap->SetResource(tex); }
	void SetOutputMap(ID3D11UnorderedAccessView* tex) { OutputMap->SetUnorderedAccessView(tex); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region DOFEffect
class DOFEffect : public Effect
{
public:
	DOFEffect(ID3D11Device* device, const std::wstring& filename);
	~DOFEffect();

public:
	ID3DX11EffectTechnique* DOFTech;

protected:
	ID3DX11EffectShaderResourceVariable* Screen;
	ID3DX11EffectShaderResourceVariable* BlurScreen;
	ID3DX11EffectShaderResourceVariable* DepthMap;

	ID3DX11EffectVectorVariable* ProjValues;
	ID3DX11EffectVectorVariable* DOFFarValues;

	/*
	ID3DX11EffectScalarVariable* DepthStart;
	ID3DX11EffectScalarVariable* DepthRange;
	ID3DX11EffectScalarVariable* FarPlane;
	ID3DX11EffectScalarVariable* NearPlane;
	*/

public:
	void SetScreen(ID3D11ShaderResourceView* tex) { Screen->SetResource(tex); }
	void SetBlurScreen(ID3D11ShaderResourceView* tex) { BlurScreen->SetResource(tex); }
	void SetDepthMap(ID3D11ShaderResourceView* tex) { DepthMap->SetResource(tex); }

	void SetProjValues(const EMath::FLOAT2 v) { ProjValues->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	void SetDOFFarValues(const EMath::FLOAT2 v) { DOFFarValues->SetFloatVector(reinterpret_cast<const float*>(&v)); }

	/*
	void SetDepthStart(const float depthStart) { DepthStart->SetFloat(depthStart); }
	void SetDepthRange(const float depthRange) { DepthRange->SetFloat(depthRange); }
	void SetFarPlane(const float farPlane = 1.0f) { FarPlane->SetFloat(farPlane); }
	void SetNearPlane(const float nearPlane = 0.0f) { NearPlane->SetFloat(nearPlane); }
	*/

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region ShadowMapEffect
class ShadowMapEffect : public Effect
{
public:
	ShadowMapEffect(ID3D11Device* device, const std::wstring& filename);
	~ShadowMapEffect();

public:
	ID3DX11EffectTechnique* BuildShadowMap_BasicTech;
	ID3DX11EffectTechnique* BuildShadowMap_NormalMapTech;
	ID3DX11EffectTechnique* BuildShadowMap_SkinningTech;

	ID3DX11EffectTechnique* BuildShadowMapAlphaClipTech;

protected:
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectShaderResourceVariable* DiffuseMap;

public:
	void SetTexTransform(const EMath::FLOAT4X4& M) { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetDiffuseMap(ID3D11ShaderResourceView* tex) { DiffuseMap->SetResource(tex); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region DebugTexEffect
class DebugTexEffect : public Effect
{
public:
	DebugTexEffect(ID3D11Device* device, const std::wstring& filename);
	~DebugTexEffect();

public:
	ID3DX11EffectTechnique* ViewArgbTech;
	ID3DX11EffectTechnique* ViewRedTech;
	ID3DX11EffectTechnique* ViewGreenTech;
	ID3DX11EffectTechnique* ViewBlueTech;
	ID3DX11EffectTechnique* ViewAlphaTech;

protected:
	ID3DX11EffectShaderResourceVariable* Texture;

public:
	void SetTexture(ID3D11ShaderResourceView* tex) { Texture->SetResource(tex); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region DeferredLightingEffect
class DeferredLightingEffect : public Effect
{
public:
	DeferredLightingEffect(ID3D11Device* device, const std::wstring& filename);
	~DeferredLightingEffect();

public:
	ID3DX11EffectTechnique* LightingTech;
	ID3DX11EffectTechnique* LightingShadowTech;
	ID3DX11EffectTechnique* LightingSSAOTech;
	ID3DX11EffectTechnique* LightingShadowSSAOTech;

protected:
	// Render Target
	ID3DX11EffectShaderResourceVariable* Albedo;
	ID3DX11EffectShaderResourceVariable* MaterialRT;
	ID3DX11EffectShaderResourceVariable* PosW;
	ID3DX11EffectShaderResourceVariable* PosH;
	ID3DX11EffectShaderResourceVariable* NormalW;
	ID3DX11EffectShaderResourceVariable* ShadowPosH;
	std::vector<ID3DX11EffectShaderResourceVariable*> RenderTargetVec;

	ID3DX11EffectShaderResourceVariable* ShadowMap;
	ID3DX11EffectShaderResourceVariable* SSAOMap;

	// IBL Map
	ID3DX11EffectShaderResourceVariable* IrradianceMap;
	ID3DX11EffectShaderResourceVariable* PreFilterMap;
	ID3DX11EffectShaderResourceVariable* BrdfLUT;

	// Lights
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* PLight;
	ID3DX11EffectVariable* SLight;
	ID3DX11EffectScalarVariable* DirLightCount;
	ID3DX11EffectScalarVariable* PLightCount;
	ID3DX11EffectScalarVariable* SLightCount;

	ID3DX11EffectMatrixVariable* ViewProjTex;

public:
	void SetRenderTargets(std::vector<ID3D11ShaderResourceView*> renderTargets);
	void SetShadowMap(ID3D11ShaderResourceView* tex) { ShadowMap->SetResource(tex); }
	void SetSSAOMap(ID3D11ShaderResourceView* tex) { SSAOMap->SetResource(tex); }

	void SetEyePosW(const EMath::FLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(EMath::FLOAT3)); }
	void SetDirectionalLightDataVec(const std::vector<DirectionalLightInfo*> lights);
	void SetPointLightDataVec(const std::vector<PointLightInfo*> lights);
	void SetSpotLightDataVec(const std::vector<SpotLightInfo*> lights);

	void SetViewProjTex(const EMath::FLOAT4X4& M) { ViewProjTex->SetMatrix(reinterpret_cast<const float*>(&M)); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region SSAOEffect
class SSAOEffect : public Effect
{
public:
	SSAOEffect(ID3D11Device* device, const std::wstring& filename);
	~SSAOEffect();

	ID3DX11EffectTechnique* SSAOTech;

	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* ViewToTexSpace;
	ID3DX11EffectVectorVariable* OffsetVectors;
	ID3DX11EffectVectorVariable* FrustumCorners;
	ID3DX11EffectScalarVariable* OcclusionRadius;
	ID3DX11EffectScalarVariable* OcclusionFadeStart;
	ID3DX11EffectScalarVariable* OcclusionFadeEnd;
	ID3DX11EffectScalarVariable* SurfaceEpsilon;

	ID3DX11EffectShaderResourceVariable* NormalDepthRT;
	ID3DX11EffectShaderResourceVariable* DepthRT;
	ID3DX11EffectShaderResourceVariable* RandomVecMap;

public:
	void SetView(EMath::FLOAT4X4& M) { View->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetViewToTexSpace(EMath::FLOAT4X4& M) { ViewToTexSpace->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetOffsetVectors(const EMath::FLOAT4 v[14]) { OffsetVectors->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 14); }
	void SetFrustumCorners(const EMath::FLOAT4 v[4]) { FrustumCorners->SetFloatVectorArray(reinterpret_cast<const float*>(v), 0, 4); }
	void SetOcclusionRadius(float f) { OcclusionRadius->SetFloat(f); }
	void SetOcclusionFadeStart(float f) { OcclusionFadeStart->SetFloat(f); }
	void SetOcclusionFadeEnd(float f) { OcclusionFadeEnd->SetFloat(f); }
	void SetSurfaceEpsilon(float f) { SurfaceEpsilon->SetFloat(f); }

	void SetNormalDepthRT(ID3D11ShaderResourceView* srv) { NormalDepthRT->SetResource(srv); }
	void SetDepthRT(ID3D11ShaderResourceView* srv) { DepthRT->SetResource(srv); }
	void SetRandomVecMap(ID3D11ShaderResourceView* srv) { RandomVecMap->SetResource(srv); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region SSAOBlurEffect
class SSAOBlurEffect : public Effect
{
public:
	SSAOBlurEffect(ID3D11Device* device, const std::wstring& filename);
	~SSAOBlurEffect();

	ID3DX11EffectTechnique* HorzBlurTech;
	ID3DX11EffectTechnique* VertBlurTech;

	ID3DX11EffectScalarVariable* TexelWidth;
	ID3DX11EffectScalarVariable* TexelHeight;

	ID3DX11EffectShaderResourceVariable* NormalDepthRT;
	ID3DX11EffectShaderResourceVariable* DepthRT;
	ID3DX11EffectShaderResourceVariable* InputImage;

public:
	void SetTexelWidth(float f) { TexelWidth->SetFloat(f); }
	void SetTexelHeight(float f) { TexelHeight->SetFloat(f); }

	void SetInputImage(ID3D11ShaderResourceView* srv) { InputImage->SetResource(srv); }

	void SetNormalDepthRT(ID3D11ShaderResourceView* srv) { NormalDepthRT->SetResource(srv); }
	void SetDepthRT(ID3D11ShaderResourceView* srv) { DepthRT->SetResource(srv); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region MixTextureEffect
class MixTextureEffect : public Effect
{
public:
	MixTextureEffect(ID3D11Device* device, const std::wstring& filename);
	~MixTextureEffect();

	ID3DX11EffectTechnique* MixTextureTech;
	ID3DX11EffectTechnique* MixTexture_AccumulateTech;
	ID3DX11EffectTechnique* MixTexture_MagicEyeTech;
	ID3DX11EffectTechnique* MixTexture_NotMagicEyeTech;
	ID3DX11EffectTechnique* MixTexture_FadeInOutTech;
	ID3DX11EffectTechnique* MixTexture_MixBloomTech;

protected:
	ID3DX11EffectShaderResourceVariable* Texture_1;
	ID3DX11EffectShaderResourceVariable* Texture_2;
	ID3DX11EffectShaderResourceVariable* Texture_3;
	ID3DX11EffectScalarVariable* Per;
	ID3DX11EffectScalarVariable* FadePer;

public:
	void SetTexture_1(ID3D11ShaderResourceView* tex) { Texture_1->SetResource(tex); }
	void SetTexture_2(ID3D11ShaderResourceView* tex) { Texture_2->SetResource(tex); }
	void SetTexture_3(ID3D11ShaderResourceView* tex) { Texture_3->SetResource(tex); }
	void SetPer(float f) { Per->SetFloat(f); }
	void SetFadePer(float f) { FadePer->SetFloat(f); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region FXAAEffect
class FXAAEffect : public Effect
{
public:
	FXAAEffect(ID3D11Device* device, const std::wstring& filename);
	~FXAAEffect();

	ID3DX11EffectTechnique* FXAATech;

protected:
	ID3DX11EffectShaderResourceVariable* Texture;
	ID3DX11EffectVectorVariable* RCPFrame;

public:
	void SetTexture(ID3D11ShaderResourceView* tex) { Texture->SetResource(tex); }
	void SetRCPFrame(EMath::FLOAT4 f4) { RCPFrame->SetFloatVector(reinterpret_cast<const float*>(&f4)); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region OutlineEffect
class OutlineEffect : public Effect
{
public:
	OutlineEffect(ID3D11Device* device, const std::wstring& filename);
	~OutlineEffect();

	ID3DX11EffectTechnique* Basic_DepthTech;
	ID3DX11EffectTechnique* NormalMap_DepthTech;
	ID3DX11EffectTechnique* Skinning_DepthTech;

	ID3DX11EffectTechnique* Basic_OverSizeTech;
	ID3DX11EffectTechnique* NormalMap_OverSizeTech;
	ID3DX11EffectTechnique* Skinning_OverSizeTech;

	ID3DX11EffectTechnique* Basic_NormalTech;
	ID3DX11EffectTechnique* NormalMap_NormalTech;
	ID3DX11EffectTechnique* Skinning_NormalTech;

	ID3DX11EffectTechnique* Outline_LaplacianTech;

	ID3DX11EffectTechnique* Mix_LaplacianTech;
	ID3DX11EffectTechnique* Mix_OverSizeTech;

protected:
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* BoneTransforms;

	ID3DX11EffectScalarVariable* Size;

protected:
	ID3DX11EffectShaderResourceVariable* NormalW;
	ID3DX11EffectShaderResourceVariable* Final;
	ID3DX11EffectShaderResourceVariable* Outline;
	ID3DX11EffectShaderResourceVariable* DepthMap;

public:
	void SetWorldInvTranspose(EMath::FLOAT4X4& M) { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetNormalW(ID3D11ShaderResourceView* tex) { NormalW->SetResource(tex); }
	void SetFinal(ID3D11ShaderResourceView* tex) { Final->SetResource(tex); }
	void SetOutline(ID3D11ShaderResourceView* tex) { Outline->SetResource(tex); }
	void SetDepthMap(ID3D11ShaderResourceView* tex) { DepthMap->SetResource(tex); }

	void SetBoneTransforms(const std::vector<EMath::FLOAT4X4> boneTransformVec);
	void SetSize(const float& size) { Size->SetFloat(size); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region MotionBlurEffect
class MotionBlurEffect : public Effect
{
public:
	MotionBlurEffect(ID3D11Device* device, const std::wstring& filename);
	~MotionBlurEffect();

	ID3DX11EffectTechnique* MotionBlurTech;

protected:
	ID3DX11EffectMatrixVariable* ViewProjInv;
	ID3DX11EffectMatrixVariable* ViewProj_Prev;

protected:
	ID3DX11EffectShaderResourceVariable* Final;
	ID3DX11EffectShaderResourceVariable* DepthMap;

public:
	void SetViewProjInv(EMath::FLOAT4X4& M) { ViewProjInv->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetViewProj_Prev(const EMath::FLOAT4X4& M) { ViewProj_Prev->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetDepthMap(ID3D11ShaderResourceView* tex) { DepthMap->SetResource(tex); }
	void SetFinal(ID3D11ShaderResourceView* tex) { Final->SetResource(tex); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region ToneMappingEffect
class ToneMappingEffect : public Effect
{
public:
	ToneMappingEffect(ID3D11Device* device, const std::wstring& filename);
	~ToneMappingEffect();

	ID3DX11EffectTechnique* ToneMapping_Simple_Tech;
	ID3DX11EffectTechnique* ToneMapping_Simple_Invert_Tech;

	ID3DX11EffectTechnique* ToneMapping_ACES_Tech;
	ID3DX11EffectTechnique* ToneMapping_Reinhard_Tech;

protected:
	ID3DX11EffectShaderResourceVariable* Input;

public:
	void SetTexture(ID3D11ShaderResourceView* tex) { Input->SetResource(tex); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region EmissiveEffect
class EmissiveEffect : public Effect
{
public:
	EmissiveEffect(ID3D11Device* device, const std::wstring& filename);
	~EmissiveEffect();

	ID3DX11EffectTechnique* BasicTech;
	ID3DX11EffectTechnique* NormalMapTech;
	ID3DX11EffectTechnique* SkinningTech;

	ID3DX11EffectTechnique* MixTech;

protected:
	ID3DX11EffectShaderResourceVariable* EmissiveMap;
	ID3DX11EffectShaderResourceVariable* Final;
	ID3DX11EffectShaderResourceVariable* EmissiveBluredOutput;
	ID3DX11EffectMatrixVariable* BoneTransforms;

public:
	void SetEmissiveMap(ID3D11ShaderResourceView* tex) { EmissiveMap->SetResource(tex); }
	void SetFinal(ID3D11ShaderResourceView* tex) { Final->SetResource(tex); }
	void SetEmissiveBluredOutput(ID3D11ShaderResourceView* tex) { EmissiveBluredOutput->SetResource(tex); }
	void SetBoneTransforms(const std::vector<EMath::FLOAT4X4> boneTransformVec);

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region BloomEffect
class BloomEffect : public Effect
{
public:
	BloomEffect(ID3D11Device* device, const std::wstring& filename);
	~BloomEffect();

	ID3DX11EffectTechnique* BloomCurveTech;

protected:
	ID3DX11EffectShaderResourceVariable* Texture;
	ID3DX11EffectScalarVariable* Threshold;

public:
	void SetTexture(ID3D11ShaderResourceView* tex) { Texture->SetResource(tex); }
	void SetThreshold(const float& th) { Threshold->SetFloat(th); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region ParticleEffect
class ParticleEffect : public Effect
{
public:
	ParticleEffect(ID3D11Device* device, const std::wstring& filename);
	~ParticleEffect();

	ID3DX11EffectTechnique* StreamOutTech;
	ID3DX11EffectTechnique* DrawTech;

private:
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectScalarVariable* GameTime;
	ID3DX11EffectScalarVariable* TimeStep;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* EmitPosW;
	ID3DX11EffectVectorVariable* EmitDirW;
	ID3DX11EffectShaderResourceVariable* TexArray;
	ID3DX11EffectShaderResourceVariable* RandomTex;

public:
	void SetViewProj(EMath::Matrix M) { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetGameTime(float f) { GameTime->SetFloat(f); }
	void SetTimeStep(float f) { TimeStep->SetFloat(f); }

	void SetEyePosW(const EMath::FLOAT3& v) { EyePosW->SetRawValue(&v, 0, sizeof(EMath::FLOAT3)); }
	void SetEmitPosW(const EMath::FLOAT3& v) { EmitPosW->SetRawValue(&v, 0, sizeof(EMath::FLOAT3)); }
	void SetEmitDirW(const EMath::FLOAT3& v) { EmitDirW->SetRawValue(&v, 0, sizeof(EMath::FLOAT3)); }

	void SetTexArray(ID3D11ShaderResourceView* tex) { TexArray->SetResource(tex); }
	void SetRandomTex(ID3D11ShaderResourceView* tex) { RandomTex->SetResource(tex); }

protected:
	virtual void GetTech() override;
};
#pragma endregion

#pragma region Effects
class Effects
{
public:
	static ColorEffect* ColorFX;

	static BasicEffect* BasicFX;
	static NormalMapEffect* NormalMapFX;
	static SkinningEffect* SkinningFX;

	static SkyEffect* SkyFX;

	static BlurEffect* BlurFX;
	static DOFEffect* DofFX;

	static ShadowMapEffect* ShadowMapFX;
	static DebugTexEffect* DebugTexFX;

	static DeferredLightingEffect* DeferredLightingFX;
	static DeferredLightingEffect* DeferredLightingPBRFX;

	static SSAOEffect* SSAOFX;
	static SSAOBlurEffect* SSAOBlurFX;

	static MixTextureEffect* MixTextureFX;
	static FXAAEffect* FXAAFX;

	static OutlineEffect* OutlineFX;
	static MotionBlurEffect* MotionBlurFX;
	static ToneMappingEffect* ToneMappingFX;
	static EmissiveEffect* EmissiveFX;
	static BloomEffect* BloomFX;

	static ParticleEffect* FireFX;

private:
	// Light, Shadow를 Set해줘야하는 Effect들
	static std::vector<BasicEffect*> BasicEffectVec;

public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

public:
	static void SetEyePos(const EMath::FLOAT3& v);

	static void SetDirectionalLightVec(const std::vector<DirectionalLightInfo*> lights);
	static void SetPointLightVec(const std::vector<PointLightInfo*> lights);
	static void SetSpotLightVec(const std::vector<SpotLightInfo*> lights);

	static void SetShadowMap(ID3D11ShaderResourceView* shadowMap);
};
#pragma endregion

#endif // EFFECTS_H