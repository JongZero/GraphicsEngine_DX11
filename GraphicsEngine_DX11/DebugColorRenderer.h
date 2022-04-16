#pragma once
#include <memory>
#include "EMath.h"

class DX11Core;
class MeshBuilder;
class ResourceManager;
struct Shared_ObjectData;
struct ID3D11DeviceContext;
struct ObjectMeshData;
struct Shared_RenderingData;

/// <summary>
/// 디버그 오브젝트를 그리는 렌더러
/// 컬러 패스를 타는 오브젝트는 디퍼드 및 조명 계산을 할 필요가 없으므로
/// 디퍼드 렌더링이 끝난 후 포워드 렌더링으로 오브젝트를 그린다.
/// 2021. 12. 29 정종영
/// </summary>
class DebugColorRenderer
{
public:
	DebugColorRenderer(std::shared_ptr<DX11Core> dx11Core, MeshBuilder* pMB, std::shared_ptr<ResourceManager> rm);
	~DebugColorRenderer();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_pResourceManager;
	ObjectMeshData* m_pAxis;
	ObjectMeshData* m_pGrid;

public:
	void StartRender(Shared_RenderingData* dataForRender);

private:
	void Render(const Shared_ObjectData* objDataForRender, ID3D11DeviceContext* dc, 
		const EMath::Matrix& view, const EMath::Matrix& proj);
	void DrawIndexed(const ObjectMeshData* myObjectMeshData, const ObjectMeshData* debugObjMeshData, 
		ID3D11DeviceContext* dc, const EMath::Matrix& objWorld, const EMath::Matrix& view,
		const EMath::Matrix& proj, bool isDepthCheck);
};

