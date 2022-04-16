#include "BaseMeshRenderer.h"
#include "ObjectMeshData.h"
#include "ResourceManager.h"
#include "RenderStates.h"
#include "DX11Core.h"
#include "Vertex.h"
#include "Effects.h"
#include "MathConverter.h"
#include "Mathhelper.h"

BaseMeshRenderer::BaseMeshRenderer(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm)
	: m_DX11Core(dx11Core), m_pResourceManager(rm)
{

}

BaseMeshRenderer::~BaseMeshRenderer()
{

}

ID3DX11EffectTechnique* BaseMeshRenderer::Render(const EMath::Matrix& view, const EMath::Matrix& proj,
	const EMath::Vector3& cameraPos, const Shared_ObjectData* objDataForRender, const EMath::Matrix& shadowTransform)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();

	// 오브젝트의 메쉬 데이터를 ID를 통해 리소스 매니저로부터 가져온다.
	ObjectMeshData* _objMeshData = m_pResourceManager->GetObjectMeshData(objDataForRender->m_Name);

	if (_objMeshData == nullptr)
		return nullptr;

	// 텍스쳐도 가져온다.
	ID3D11ShaderResourceView* _diffuseMap = m_pResourceManager->GetDiffuseMap(objDataForRender->m_DiffuseMapName);
	ID3D11ShaderResourceView* _normalMap = m_pResourceManager->GetNormalMap(objDataForRender->m_NormalMapName);

	ID3D11ShaderResourceView* _metallicMap = m_pResourceManager->GetMetallicMap(objDataForRender->m_DiffuseMapName);
	ID3D11ShaderResourceView* _roughnessMap = m_pResourceManager->GetRoughnessMap(objDataForRender->m_DiffuseMapName);

	bool _isDiffuseMapUsed = false;
	bool _isNormalMapUsed = false;

	bool _isMetallicMapUsed = false;
	bool _isRoughnessMapUsed = false;

	// DiffuseMap이 사용되는지?
	if (_diffuseMap != nullptr)
	{
		_isDiffuseMapUsed = true;

		// NormalMap이 사용되는지?
		if (_normalMap != nullptr)
		{
			_isNormalMapUsed = true;
		}
	}

	// PBR, Metallic Map과 Roughness Map 사용 여부
	if (_metallicMap != nullptr)
		_isMetallicMapUsed = true;

	if (_roughnessMap != nullptr)
		_isRoughnessMapUsed = true;

	/// 렌더스테이트 설정
	switch (_objMeshData->m_ObjectMeshInfo.m_RenderStateType)
	{
	case ObjectMeshInfo::eRenderStateType::Solid:
		_dc->RSSetState(RenderStates::GetSolidRS());
		break;
	case ObjectMeshInfo::eRenderStateType::Wire:
		_dc->RSSetState(RenderStates::GetWireframeRS());
		break;
	case ObjectMeshInfo::eRenderStateType::Now:
		_dc->RSSetState(RenderStates::GetNowRS());
		break;
	}

	/// InputLayout 설정
	ID3D11InputLayout* _inputLayout = nullptr;

	switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
	{
	case ObjectMeshInfo::eEffectType::Color:
		_inputLayout = InputLayouts::PosColor;
		break;
	case ObjectMeshInfo::eEffectType::Basic:
		_inputLayout = InputLayouts::PosNormalTex;
		break;
	case ObjectMeshInfo::eEffectType::NormalMap:
		_inputLayout = InputLayouts::PosNormalTexTangentU;
		break;
	case ObjectMeshInfo::eEffectType::Skinning:
		_inputLayout = InputLayouts::PosNormalTexTangentUWeightsBoneIndices;
		break;
	}

	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	assert(_inputLayout);
	_dc->IASetInputLayout(_inputLayout);

	UINT offset = 0;
	_dc->IASetVertexBuffers(0, 1, _objMeshData->m_VertexBuffer.GetAddressOf(), &_objMeshData->m_Stride, &offset);
	_dc->IASetIndexBuffer(_objMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	/// Set Active Tech
	ID3DX11EffectTechnique* _activeTech = nullptr;

	if (_objMeshData->m_ObjectMeshInfo.m_EffectType != ObjectMeshInfo::eEffectType::Color)
	{
		/// 알파가 있는지 없는지 체크
		RenderType rt;
		if (objDataForRender->m_LegacyMaterialData.Diffuse.w < 1.0f)
			rt = RenderType::Forward;
		else
			rt = RenderType::Deferred;

		BasicEffect* effect = nullptr;
		switch (_objMeshData->m_ObjectMeshInfo.m_EffectType)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			_activeTech = SetBasicTech(rt, _isDiffuseMapUsed);
			effect = Effects::BasicFX;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			_activeTech = SetNormalMapTech(rt);
			effect = Effects::NormalMapFX;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			_activeTech = SetSkinningTech(rt, _isDiffuseMapUsed, _isNormalMapUsed);
			effect = Effects::SkinningFX;
			break;
		}

		assert(effect);

		effect->SetMaterial(objDataForRender->m_LegacyMaterialData);

		// Texture
		effect->SetTexTransform(objDataForRender->m_TexTransform);

		// Diffuse Map이 있다면 현재 인덱스에 맞게 세팅
		if (_isDiffuseMapUsed)
		{
			effect->SetDiffuseMap(_diffuseMap);
		}

		// Normal Map이 있다면 현재 인덱스에 맞게 세팅
		if (_isNormalMapUsed)
		{
			dynamic_cast<NormalMapEffect*>(effect)->SetNormalMap(_normalMap);
		}

		if (_isMetallicMapUsed)
		{
			effect->SetMetallicMap(_metallicMap);
			effect->SetMetallicFLOAT(-1);
		}

		if (_isRoughnessMapUsed)
		{
			effect->SetRoughnessMap(_roughnessMap);
			effect->SetRoughnessFLOAT(-1);
		}
	}
	else
		_activeTech = Effects::ColorFX->ColorTech;

	return _activeTech;
}

void BaseMeshRenderer::SetAllTM(const ObjectMeshInfo::eEffectType& et, const EMath::Matrix& meshWorld, const EMath::Matrix& view,
	const EMath::Matrix& proj, const EMath::Matrix& shadowTransform)
{
	// Effect 공통
	EMath::Matrix worldViewProj = meshWorld * view * proj;

	if (et != ObjectMeshInfo::eEffectType::Color)
	{
		EMath::Matrix worldInvTranspose = MathHelper::InverseTranspose(meshWorld);

		BasicEffect* effect = nullptr;
		switch (et)
		{
		case ObjectMeshInfo::eEffectType::Basic:
			effect = Effects::BasicFX;
			break;
		case ObjectMeshInfo::eEffectType::NormalMap:
			effect = Effects::NormalMapFX;
			break;
		case ObjectMeshInfo::eEffectType::Skinning:
			effect = Effects::SkinningFX;
			break;
		}

		assert(effect);

		effect->SetWorld(meshWorld);
		effect->SetWorldView(meshWorld * view);
		effect->SetWorldInvTranspose(worldInvTranspose);
		effect->SetWorldInvTransposeView(worldInvTranspose * view);
		effect->SetWorldViewProj(worldViewProj);
		effect->SetViewProj(view * proj);

		/// Shadow
		effect->SetShadowTransform(meshWorld * shadowTransform);
	}
	else
		Effects::ColorFX->SetWorldViewProj(worldViewProj);
}

ID3DX11EffectTechnique* BaseMeshRenderer::SetBasicTech(RenderType rt, bool isDiffuseMapExist)
{
	ID3DX11EffectTechnique* _activeTech = nullptr;

	switch (rt)
	{
	case BaseMeshRenderer::RenderType::Forward:
	{
		if (isDiffuseMapExist)
			_activeTech = Effects::BasicFX->ForwardTexTech;
		else
			_activeTech = Effects::BasicFX->ForwardTech;
	}
	break;
	case BaseMeshRenderer::RenderType::Deferred:
	{
		if (isDiffuseMapExist)
			_activeTech = Effects::BasicFX->DeferredTexTech;
		else
			_activeTech = Effects::BasicFX->DeferredTech;
	}
	break;
	}

	return _activeTech;
}

ID3DX11EffectTechnique* BaseMeshRenderer::SetNormalMapTech(RenderType rt)
{
	ID3DX11EffectTechnique* _activeTech = nullptr;

	switch (rt)
	{
	case BaseMeshRenderer::RenderType::Forward:
	{
		_activeTech = Effects::NormalMapFX->ForwardTexNormalMapTech;
	}
	break;
	case BaseMeshRenderer::RenderType::Deferred:
	{
		_activeTech = Effects::NormalMapFX->DeferredTexNormalMapTech;
	}
	break;
	}

	return _activeTech;
}

ID3DX11EffectTechnique* BaseMeshRenderer::SetSkinningTech(RenderType rt, bool isDiffuseMapExist, bool isNormalMapExist)
{
	ID3DX11EffectTechnique* _activeTech = nullptr;

	switch (rt)
	{
	case BaseMeshRenderer::RenderType::Forward:
	{
		if (isDiffuseMapExist)
		{
			if (isNormalMapExist)
			{
				_activeTech = Effects::SkinningFX->ForwardTexNormalMapTech;
			}
			else
			{
				_activeTech = Effects::SkinningFX->ForwardTexTech;
			}
		}
		else
		{
			_activeTech = Effects::SkinningFX->ForwardTech;
		}
	}
	break;
	case BaseMeshRenderer::RenderType::Deferred:
	{
		if (isDiffuseMapExist)
		{
			if (isNormalMapExist)
			{
				_activeTech = Effects::SkinningFX->DeferredTexNormalMapTech;
			}
			else
			{
				_activeTech = Effects::SkinningFX->DeferredTexTech;
			}
		}
		else
		{
			_activeTech = Effects::SkinningFX->DeferredTech;
		}
	}
	break;
	}
	
	return _activeTech;
}
