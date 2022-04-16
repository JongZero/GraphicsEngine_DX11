#pragma once
#include "GeometryGenerator.h"
#include "ObjectMeshInfo.h"
#include "ObjectMeshData.h"
#include "ErrChecker.h"
#include "ParserData.h"
#include "DX11Core.h"

#include <memory>

class ResourceManager;
class GeometryGenerator;

/// <summary>
/// GeometryGenerator에서 생성하거나 ASE, FBX 파일을 로드해 읽어들인 Vertex, Index 정보를 이 클래스에서 
/// 실질적으로 버퍼로 만든다. 만든 후에는 ResourceManager에 이름과 함께 저장하고, 후에 게임엔진으로부터 이름를 받아와
/// ResourceManager 내부에 이름과 비교하여 이름이 있으면 그린다.
/// 2021. 10. 27 정종영
/// </summary>
class MeshBuilder
{
public:
	MeshBuilder(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	~MeshBuilder();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	std::unique_ptr<GeometryGenerator> m_GeometryGenerator;

private:
	// Picking용 BoundingBox를 위한 체크 변수
	ObjectMeshData* m_MyObjectMeshData;

public:
	// 지오메트리를 직접 생성했을경우
	void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo);

private:
	// FBXParser로부터 데이터를 가져와 생성할 경우
	void CreateMeshes_FromFBX(std::wstring fileName);

private:
	// 지오메트리를 직접 생성했을경우
	void CopyGeometryDataAndCreateBuffers(ObjectMeshData* pObjectMeshData, GeometryGenerator::GeoData& meshData, bool isPickingMesh = false);

	// FBXParser로부터 데이터를 받을 경우
	void CopyGeometryDataAndCreateBuffers(ObjectMeshData* pObjectMeshData, std::vector<ParserData::Mesh*> fbxMeshVec);

	void LoadIndices(ObjectMeshData* pObjectMeshData, std::vector<ParserData::Mesh*> fbxMeshVec);

	void MakeTangent(std::vector<ParserData::Mesh*> meshDataVec);

	template<typename T> void CreateBuffers(ObjectMeshData* pObjectMeshData, std::vector<T>& vertices);

	// Picking을 위한 BoundingBox 생성
	void CreateBoundingBoxForPicking(std::wstring objName, ObjectMeshData* pObjectMeshData);
};

template<typename T>
void MeshBuilder::CreateBuffers(ObjectMeshData* pObjectMeshData, std::vector<T>& vertices)
{
	pObjectMeshData->m_Stride = sizeof(T);

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = pObjectMeshData->m_Stride * (UINT)pObjectMeshData->m_VertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(m_DX11Core->GetDevice()->CreateBuffer(&vbd, &vinitData, pObjectMeshData->m_VertexBuffer.GetAddressOf()));

	size_t size = pObjectMeshData->m_IndexVec.size();

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * (UINT)pObjectMeshData->m_IndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &pObjectMeshData->m_IndexVec[0];
	HR(m_DX11Core->GetDevice()->CreateBuffer(&ibd, &iinitData, pObjectMeshData->m_IndexBuffer.GetAddressOf()));
}