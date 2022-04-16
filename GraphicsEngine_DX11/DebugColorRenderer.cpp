#include "DebugColorRenderer.h"
#include "DX11Core.h"
#include "RenderStates.h"
#include "Shared_RenderingData.h"
#include "Vertex.h"
#include "ObjectMeshData.h"
#include "ResourceManager.h"
#include "Effects.h"
#include "Mesh.h"
#include "MeshBuilder.h"

DebugColorRenderer::DebugColorRenderer(std::shared_ptr<DX11Core> dx11Core, MeshBuilder* pMB, std::shared_ptr<ResourceManager> rm)
	: m_DX11Core(dx11Core), m_pResourceManager(rm)
{
	ObjectMeshInfo axis(ObjectMeshInfo::eMeshType::Axis,
		ObjectMeshInfo::eRenderStateType::Wire, ObjectMeshInfo::eEffectType::Color,
		EMath::Vector4(0, 0, 0, 0), EMath::Vector3(1000, 0, 0));
	pMB->CreateMeshes(L"Axis", axis);

	ObjectMeshInfo grid(ObjectMeshInfo::eMeshType::Grid,
		ObjectMeshInfo::eRenderStateType::Wire, ObjectMeshInfo::eEffectType::Color,
		EMath::Vector4(0, 0, 0, 0), EMath::Vector3(1000, 0, 0));
	pMB->CreateMeshes(L"Grid", grid);

	m_pAxis = rm->GetObjectMeshData(L"Axis");
	m_pGrid = rm->GetObjectMeshData(L"Grid");
}

DebugColorRenderer::~DebugColorRenderer()
{
	// Axis와 Grid는 리소스매니저에서 소유권을 갖고있고, 
	// 그 쪽에서 해제할 것이므로 이 곳에서 해제하지 않는다.
}

void DebugColorRenderer::StartRender(Shared_RenderingData* dataForRender)
{
	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();
	
	// 쉐이더 상에서 와이어프레임을 지정해놓았다.
	//_dc->RSSetState(RenderStates::GetWireframeRS());

	ID3D11InputLayout* _inputLayout = InputLayouts::PosColor;
	_dc->IASetInputLayout(_inputLayout);

	// Axis, Grid는 라인으로 그려줌
	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	DrawIndexed(m_pAxis, m_pAxis, _dc, EMath::Matrix::Identity, dataForRender->m_View, dataForRender->m_Proj, true);
	DrawIndexed(m_pGrid, m_pGrid, _dc, EMath::Matrix::Identity, dataForRender->m_View, dataForRender->m_Proj, true);

	// 나머지는 다시 원래대로
	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (const auto& it : dataForRender->m_ObjectDataForRenderVec)
	{
		Render(it, _dc, dataForRender->m_View, dataForRender->m_Proj);
	}
}

void DebugColorRenderer::Render(const Shared_ObjectData* objDataForRender, ID3D11DeviceContext* dc, 
	const EMath::Matrix& view, const EMath::Matrix& proj)
{
	ObjectMeshData* objMeshData = m_pResourceManager->GetObjectMeshData(objDataForRender->m_Name);

	if (objMeshData)
	{
		ObjectMeshData* debugObject = objMeshData->m_BoundingBoxForPickingMeshData.get();

		if (debugObject)
		{
			DrawIndexed(objMeshData, debugObject, dc, objDataForRender->m_World, view, proj, false);
		}
	}
}

void DebugColorRenderer::DrawIndexed(const ObjectMeshData* myObjectMeshData, const ObjectMeshData* debugObjMeshData,
	ID3D11DeviceContext* dc, const EMath::Matrix& objWorld, const EMath::Matrix& view,
	const EMath::Matrix& proj, bool isDepthCheck)
{
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, debugObjMeshData->m_VertexBuffer.GetAddressOf(), &debugObjMeshData->m_Stride, &offset);
	dc->IASetIndexBuffer(debugObjMeshData->m_IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	ID3DX11EffectTechnique* _activeTech = nullptr;
	if (isDepthCheck)
		_activeTech = Effects::ColorFX->ColorTech;
	else
		_activeTech = Effects::ColorFX->Color_NoDepthTech;

	D3DX11_TECHNIQUE_DESC techDesc;
	_activeTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (UINT m = 0; m < debugObjMeshData->m_MeshVec.size(); m++)
		{
			EMath::Matrix meshWorld = myObjectMeshData->m_MeshVec[m]->WorldTM;
			//meshWorld *= myObjectMeshData->m_MeshVec[m]->NodeTM;
			
			meshWorld = meshWorld * objWorld;

			EMath::Matrix worldViewProj = meshWorld * view * proj;
			Effects::ColorFX->SetWorldViewProj(worldViewProj);

			_activeTech->GetPassByIndex(p)->Apply(0, dc);
			dc->DrawIndexed(debugObjMeshData->m_MeshVec[m]->IndexCount,
				debugObjMeshData->m_MeshVec[m]->StartIndex,
				0);
		}
	}
}
