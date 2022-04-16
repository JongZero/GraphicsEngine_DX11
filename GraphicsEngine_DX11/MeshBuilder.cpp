#include "MeshBuilder.h"
#include "Mesh.h"
#include "GeometryGenerator.h"
#include "MathConverter.h"
#include "ResourceManager.h"
#include "Vertex.h"

#include <MacroDefine.h>

MeshBuilder::MeshBuilder(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm)
	: m_DX11Core(dx11Core), m_ResourceManager(rm), m_MyObjectMeshData(nullptr)
{
	m_GeometryGenerator = std::make_unique<GeometryGenerator>();
}

MeshBuilder::~MeshBuilder()
{
}

void MeshBuilder::CreateMeshes(std::wstring objName, ObjectMeshInfo& omi)
{
	// GeometryGenerator를 통해 지오메트리 데이터를 가져올 변수
	GeometryGenerator::GeoData _meshData;

	switch (omi.m_MeshType)
	{
	case ObjectMeshInfo::eMeshType::Box:
		if (omi.m_Center == EMath::Vector3(0, 0, 0))
			m_GeometryGenerator->CreateBox(omi.m_Scale.x / 2, omi.m_Scale.y / 2, omi.m_Scale.z / 2, _meshData);
		else
			m_GeometryGenerator->CreateBox(omi.m_Center, omi.m_Scale / 2, _meshData);
		break;
	case ObjectMeshInfo::eMeshType::ColorBox:
		if (omi.m_Center == EMath::Vector3(0, 0, 0))
			m_GeometryGenerator->CreateBox(omi.m_Scale.x, omi.m_Scale.y, omi.m_Scale.z, _meshData);
		else
			m_GeometryGenerator->CreateBox(omi.m_Center, omi.m_Scale, _meshData);
		break;
	case ObjectMeshInfo::eMeshType::Plane:
		if (omi.m_Scale == EMath::Vector3(1, 1, 1))
			m_GeometryGenerator->CreateBox(10.0f, 0.01f, 10.0f, _meshData);
		else
			m_GeometryGenerator->CreateBox(omi.m_Scale.x, omi.m_Scale.y, omi.m_Scale.z, _meshData);
		break;
	case ObjectMeshInfo::eMeshType::Sphere:
		if (omi.m_Scale == EMath::Vector3(1, 1, 1))
			m_GeometryGenerator->CreateSphere(0.5f, 20, 20, _meshData);
		else
			m_GeometryGenerator->CreateSphere(omi.m_Scale.x, (UINT)omi.m_Scale.y, (UINT)omi.m_Scale.z, _meshData);
		break;
	case ObjectMeshInfo::eMeshType::Geosphere:
		if (omi.m_Scale == EMath::Vector3(1, 1, 1))
			m_GeometryGenerator->CreateGeosphere(0.5f, 20, _meshData);
		else
			m_GeometryGenerator->CreateGeosphere(omi.m_Scale.x, (UINT)omi.m_Scale.y, _meshData);
		break;
	case ObjectMeshInfo::eMeshType::Cylinder:
		if (omi.m_Scale == EMath::Vector3(1, 1, 1))
			m_GeometryGenerator->CreateCylinder(0.5f, 0.3f, 3.0f, 20, 20, _meshData);
		else
			m_GeometryGenerator->CreateCylinder(omi.m_Scale.x, omi.m_Scale.y, omi.m_Scale.z, 20, 20, _meshData);
		break;
	case ObjectMeshInfo::eMeshType::Skull:
		m_GeometryGenerator->CreateSkull(_meshData);
		break;
	case ObjectMeshInfo::eMeshType::Grid:
		if (omi.m_Scale == EMath::Vector3(1, 1, 1))
			m_GeometryGenerator->CreateGrid(10, _meshData);
		else
			m_GeometryGenerator->CreateGrid((size_t)omi.m_Scale.x, _meshData);
		break;
	case ObjectMeshInfo::eMeshType::Axis:
		if (omi.m_Scale == EMath::Vector3(1, 1, 1))
			m_GeometryGenerator->CreateAxis(10.0f, _meshData);
		else
			m_GeometryGenerator->CreateAxis(omi.m_Scale.x, _meshData);
		break;
	//case ObjectMeshInfo::eMeshType::Frustum:
		//m_GeometryGenerator->CreateFrustum(m_View, m_Proj, _meshData);
	//	break;
	case ObjectMeshInfo::eMeshType::FullScreenQuad:
		m_GeometryGenerator->CreateFullscreenQuad(_meshData);
		break;
	case ObjectMeshInfo::eMeshType::Cube:
		m_GeometryGenerator->CreateCube(_meshData);
		break;
	case ObjectMeshInfo::eMeshType::FBXERJ:
		CreateMeshes_FromFBX(objName);
		return;
	default:
		break;
	}

	// GeometryGenerator를 통해 지오메트리 데이터가 생성 되었다면
	if (_meshData.Indices.size() > 0)
	{
		// 오브젝트 한 개의 메쉬 데이터들 생성, 오브젝트 메쉬의 정보를 저장해놓음
		std::unique_ptr<ObjectMeshData> omd = std::make_unique<ObjectMeshData>(omi);
		
		// 지오메트리 데이터를 복사 후 버퍼를 생성
		CopyGeometryDataAndCreateBuffers(omd.get(), _meshData);

		// Solid일 경우에만 Picking용 BoundingBox 생성
		if (omi.m_RenderStateType != ObjectMeshInfo::eRenderStateType::Wire
			&& omi.m_MeshType != ObjectMeshInfo::eMeshType::FullScreenQuad)
		{
			CreateBoundingBoxForPicking(objName, omd.get());
		}
		else
		{
			// 현재 생성되는 메쉬가 Picking을 위한 BoundingBox일 경우
			if (omi.m_MeshType == ObjectMeshInfo::eMeshType::ColorBox && m_MyObjectMeshData)
			{
				m_MyObjectMeshData->m_BoundingBoxForPickingMeshData = std::move(omd);
				m_MyObjectMeshData = nullptr;
			}
		}

		// 리소스매니저에 ID와 함께 보관
		m_ResourceManager->AddObjectMeshData(objName, std::move(omd));
	}
}

void MeshBuilder::CreateMeshes_FromFBX(std::wstring fileName)
{
	ObjectMeshInfo _objectMeshInfo(ObjectMeshInfo::eMeshType::FBXERJ, ObjectMeshInfo::eRenderStateType::Solid, ObjectMeshInfo::eEffectType::Basic);

	std::vector<ParserData::Mesh*> _fbxMeshVec = m_ResourceManager->GetMeshData(fileName);

	// FBX 파일 이름과 같은 이름의 노말맵 파일이 있다면 노말맵을 쓰는 것으로 간주하고 노말맵에 필요한 데이터까지 버퍼로 만든다.
	if (m_ResourceManager->GetNormalMap(fileName) != nullptr)
	{
		_objectMeshInfo.m_EffectType = ObjectMeshInfo::eEffectType::NormalMap;
	}

	if (_fbxMeshVec.size() > 0)
	{
		// temp
		std::vector<Mesh*> _meshVec;
		size_t _vertexCount = 0;

		/// 메쉬 데이터 뽑아오기
		for (UINT i = 0; i < _fbxMeshVec.size(); i++)
		{
			std::unique_ptr<Mesh> newMeshData = std::make_unique<Mesh>();

			// Node Name
			newMeshData->NodeName = _fbxMeshVec[i]->m_NodeName;
			newMeshData->NodeParentName = _fbxMeshVec[i]->m_NodeParentName;

			// Node TM
			newMeshData->NodeTM = _fbxMeshVec[i]->m_NodeTM;

			// Node 역행렬
			newMeshData->NodeInverseTM = _fbxMeshVec[i]->m_NodeInverseTM;

			// StartVertex Index
			newMeshData->StartVertex = (UINT)_vertexCount;

			/// 애니메이션
			/// 스키닝
			// 본인가요?
			newMeshData->IsBone = _fbxMeshVec[i]->m_IsBone;

			// 본이라면 Index는?
 			if (newMeshData->IsBone)
 				newMeshData->BoneIndex = _fbxMeshVec[i]->m_BoneIndex;

			// 스킨드 메쉬인가요?
 			newMeshData->IsSkinnedMesh = _fbxMeshVec[i]->m_IsSkinnedMesh;

			// 스킨드 메쉬가 하나 이상일 경우 스키닝 오브젝트
 			if (newMeshData->IsSkinnedMesh)
				_objectMeshInfo.m_EffectType = ObjectMeshInfo::eEffectType::Skinning;

			// Mesh Info 벡터에 넣기
			_meshVec.push_back(newMeshData.get());
			m_ResourceManager->AddUniqueMesh(std::move(newMeshData));

			_vertexCount += _fbxMeshVec[i]->m_OptVertexVec.size();
		}

		// 오브젝트 한 개의 메쉬 데이터들 생성, 오브젝트 메쉬의 정보를 저장해놓음
		std::unique_ptr<ObjectMeshData> omd = std::make_unique<ObjectMeshData>(_objectMeshInfo);
		omd->m_VertexCount = _vertexCount;
		omd->m_MeshVec.assign(_meshVec.begin(), _meshVec.end());

		CopyGeometryDataAndCreateBuffers(omd.get(), _fbxMeshVec);

		// Solid일 경우에만 Picking용 BoundingBox 생성
		if (_objectMeshInfo.m_RenderStateType != ObjectMeshInfo::eRenderStateType::Wire)
		{
			CreateBoundingBoxForPicking(fileName, omd.get());
		}

		// 리소스매니저에 이름과 함께 보관
		m_ResourceManager->AddObjectMeshData(fileName, std::move(omd));
	}
}

void MeshBuilder::CopyGeometryDataAndCreateBuffers(ObjectMeshData* pObjectMeshData, GeometryGenerator::GeoData& meshData, bool isPickingMesh)
{
	// Mesh 생성
	std::unique_ptr<Mesh> newMesh = std::make_unique<Mesh>();

	pObjectMeshData->m_VertexCount = meshData.Vertices.size();
	pObjectMeshData->m_IndexCount = meshData.Indices.size();

	newMesh->IndexCount = pObjectMeshData->m_IndexCount;

	UINT k = 0;
	pObjectMeshData->m_IndexVec.assign(meshData.Indices.begin(), meshData.Indices.end());

	switch (pObjectMeshData->m_ObjectMeshInfo.m_EffectType)
	{
	case ObjectMeshInfo::eEffectType::Color:
	{
		std::vector<Vertex::PosColor> vertices(pObjectMeshData->m_VertexCount);
		for (size_t i = 0; i < meshData.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos = meshData.Vertices[i].Position;

			// Axis 또는 Grid일 경우
			if (pObjectMeshData->m_ObjectMeshInfo.m_MeshType == ObjectMeshInfo::eMeshType::Axis
				|| pObjectMeshData->m_ObjectMeshInfo.m_MeshType == ObjectMeshInfo::eMeshType::Grid)
			{
				vertices[k].Color = meshData.Vertices[i].Color;
			}
			else
			{
				vertices[k].Color = pObjectMeshData->m_ObjectMeshInfo.m_Color;
			}

			// 버텍스의 최소, 최대 구하기
			EMath::Vector3 _pos = vertices[k].Pos;
			pObjectMeshData->m_MinVertexPos = EMath::Vector3::Min(pObjectMeshData->m_MinVertexPos, _pos);
			pObjectMeshData->m_MaxVertexPos = EMath::Vector3::Max(pObjectMeshData->m_MaxVertexPos, _pos);

			// 포지션 값 저장해놓음
			pObjectMeshData->m_VertexPosVec.push_back(_pos);
		}

		CreateBuffers(pObjectMeshData, vertices);
	}
	break;
	case ObjectMeshInfo::eEffectType::Basic:
	{
		std::vector<Vertex::PosNormalTex> vertices(pObjectMeshData->m_VertexCount);
		for (size_t i = 0; i < meshData.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos = meshData.Vertices[i].Position;
			vertices[k].Normal = meshData.Vertices[i].Normal;
			vertices[k].Tex = meshData.Vertices[i].TexC;

			// 버텍스의 최소, 최대 구하기
			EMath::Vector3 _pos = vertices[k].Pos;
			pObjectMeshData->m_MinVertexPos = EMath::Vector3::Min(pObjectMeshData->m_MinVertexPos, _pos);
			pObjectMeshData->m_MaxVertexPos = EMath::Vector3::Max(pObjectMeshData->m_MaxVertexPos, _pos);

			// 포지션 값 저장해놓음
			pObjectMeshData->m_VertexPosVec.push_back(_pos);
		}

		CreateBuffers(pObjectMeshData, vertices);
	}
	break;
	case ObjectMeshInfo::eEffectType::NormalMap:
	{
		std::vector<Vertex::PosNormalTexTangentU> vertices(pObjectMeshData->m_VertexCount);
		for (size_t i = 0; i < meshData.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos = meshData.Vertices[i].Position;
			vertices[k].Normal = meshData.Vertices[i].Normal;
			vertices[k].Tex = meshData.Vertices[i].TexC;

			// Normal Map을 위한 Tangent 추가
			vertices[k].TangentU = meshData.Vertices[i].TangentU;

			// 버텍스의 최소, 최대 구하기
			EMath::Vector3 _pos = vertices[k].Pos;
			pObjectMeshData->m_MinVertexPos = EMath::Vector3::Min(pObjectMeshData->m_MinVertexPos, _pos);
			pObjectMeshData->m_MaxVertexPos = EMath::Vector3::Max(pObjectMeshData->m_MaxVertexPos, _pos);

			// 포지션 값 저장해놓음
			pObjectMeshData->m_VertexPosVec.push_back(_pos);
		}

		CreateBuffers(pObjectMeshData, vertices);
	}
	break;
	case ObjectMeshInfo::eEffectType::Skinning:
	{
		std::vector<Vertex::PosNormalTexTangentUWeightsBoneIndices> vertices(pObjectMeshData->m_VertexCount);
		for (size_t i = 0; i < meshData.Vertices.size(); ++i, ++k)
		{
			vertices[k].Pos = meshData.Vertices[i].Position;
			vertices[k].Normal = meshData.Vertices[i].Normal;
			vertices[k].Tex = meshData.Vertices[i].TexC;

			// Normal Map을 위한 Tangent 추가
			vertices[k].TangentU = meshData.Vertices[i].TangentU;

			// 버텍스의 최소, 최대 구하기
			EMath::Vector3 _pos = vertices[k].Pos;
			pObjectMeshData->m_MinVertexPos = EMath::Vector3::Min(pObjectMeshData->m_MinVertexPos, _pos);
			pObjectMeshData->m_MaxVertexPos = EMath::Vector3::Max(pObjectMeshData->m_MaxVertexPos, _pos);

			// 포지션 값 저장해놓음
			pObjectMeshData->m_VertexPosVec.push_back(_pos);
		}

		CreateBuffers(pObjectMeshData, vertices);
	}
	break;
	}

	pObjectMeshData->m_MeshVec.push_back(newMesh.get());
	m_ResourceManager->AddUniqueMesh(std::move(newMesh));
}

void MeshBuilder::CopyGeometryDataAndCreateBuffers(ObjectMeshData* pObjectMeshData, std::vector<ParserData::Mesh*> fbxMeshVec)
{
	int i = 0;
	switch (pObjectMeshData->m_ObjectMeshInfo.m_EffectType)
	{
	case ObjectMeshInfo::eEffectType::Color:
	{
		// Vertex
		std::vector<Vertex::PosColor> vertices(pObjectMeshData->m_VertexCount);
		for (UINT j = 0; j < fbxMeshVec.size(); j++)
		{
			for (UINT k = 0; k < fbxMeshVec[j]->m_OptVertexVec.size(); k++, i++)
			{
				vertices[i].Pos = fbxMeshVec[j]->m_OptVertexVec[k].m_Pos;
				vertices[i].Color = pObjectMeshData->m_ObjectMeshInfo.m_Color;

				// 버텍스의 최소, 최대 구하기
				EMath::Vector3 _pos = vertices[k].Pos;
				pObjectMeshData->m_MinVertexPos = EMath::Vector3::Min(pObjectMeshData->m_MinVertexPos, _pos);
				pObjectMeshData->m_MaxVertexPos = EMath::Vector3::Max(pObjectMeshData->m_MaxVertexPos, _pos);

				// 포지션 값 저장해놓음
				pObjectMeshData->m_VertexPosVec.push_back(_pos);
			}
		}

		// Index
		LoadIndices(pObjectMeshData, fbxMeshVec);
		CreateBuffers(pObjectMeshData, vertices);
	}
	break;

	case ObjectMeshInfo::eEffectType::Basic:
	{
		std::vector<Vertex::PosNormalTex> vertices(pObjectMeshData->m_VertexCount);
		for (UINT j = 0; j < fbxMeshVec.size(); j++)
		{
			for (UINT k = 0; k < fbxMeshVec[j]->m_OptVertexVec.size(); k++, i++)
			{
				vertices[i].Pos = fbxMeshVec[j]->m_OptVertexVec[k].m_Pos;
				vertices[i].Normal = fbxMeshVec[j]->m_OptVertexVec[k].m_Normal;
				vertices[i].Tex = fbxMeshVec[j]->m_OptVertexVec[k].m_UV;

				// 버텍스의 최소, 최대 구하기
				EMath::Vector3 _pos = vertices[k].Pos;
				pObjectMeshData->m_MinVertexPos = EMath::Vector3::Min(pObjectMeshData->m_MinVertexPos, _pos);
				pObjectMeshData->m_MaxVertexPos = EMath::Vector3::Max(pObjectMeshData->m_MaxVertexPos, _pos);

				// 포지션 값 저장해놓음
				pObjectMeshData->m_VertexPosVec.push_back(_pos);
			}
		}

		// Index
		LoadIndices(pObjectMeshData, fbxMeshVec);
		CreateBuffers(pObjectMeshData, vertices);
	}
	break;

	case ObjectMeshInfo::eEffectType::NormalMap:
	{
		// 탄젠트 먼저 생성
		MakeTangent(fbxMeshVec);
		
		std::vector<Vertex::PosNormalTexTangentU> vertices(pObjectMeshData->m_VertexCount);

		for (UINT j = 0; j < fbxMeshVec.size(); j++)
		{
			for (UINT k = 0; k < fbxMeshVec[j]->m_OptVertexVec.size(); k++, i++)
			{
				vertices[i].Pos = fbxMeshVec[j]->m_OptVertexVec[k].m_Pos;
				vertices[i].Normal = fbxMeshVec[j]->m_OptVertexVec[k].m_Normal;
				vertices[i].Tex = fbxMeshVec[j]->m_OptVertexVec[k].m_UV;

				// 정규화하기
				EMath::Vector3 normalizedT = fbxMeshVec[j]->m_OptVertexVec[k].m_Tangent;
				normalizedT.Normalize();
				vertices[i].TangentU = normalizedT;

				// 버텍스의 최소, 최대 구하기
				EMath::Vector3 _pos = vertices[k].Pos;
				pObjectMeshData->m_MinVertexPos = EMath::Vector3::Min(pObjectMeshData->m_MinVertexPos, _pos);
				pObjectMeshData->m_MaxVertexPos = EMath::Vector3::Max(pObjectMeshData->m_MaxVertexPos, _pos);

				// 포지션 값 저장해놓음
				pObjectMeshData->m_VertexPosVec.push_back(_pos);
			}
		}

		// Index
		LoadIndices(pObjectMeshData, fbxMeshVec);
		CreateBuffers(pObjectMeshData, vertices);
	}
	break;

	case ObjectMeshInfo::eEffectType::Skinning:
	{
		// 탄젠트 먼저 생성
		MakeTangent(fbxMeshVec);

		std::vector<Vertex::PosNormalTexTangentUWeightsBoneIndices> vertices(pObjectMeshData->m_VertexCount);

		for (UINT j = 0; j < fbxMeshVec.size(); j++)
		{
			for (UINT k = 0; k < fbxMeshVec[j]->m_OptVertexVec.size(); k++, i++)
			{
				vertices[i].Pos = fbxMeshVec[j]->m_OptVertexVec[k].m_Pos;
				vertices[i].Normal = fbxMeshVec[j]->m_OptVertexVec[k].m_Normal;
				vertices[i].Tex = fbxMeshVec[j]->m_OptVertexVec[k].m_UV;

				// Weight 넣기
				vertices[i].Weights = fbxMeshVec[j]->m_OptVertexVec[k].m_BoneWeight;

				for (UINT b = 0; b < 4; b++)
				{
					// Bone Index 넣기
					switch (b)
					{
					case 0:
						vertices[i].BoneIndices[b] = (BYTE)fbxMeshVec[j]->m_OptVertexVec[k].m_BoneIndex.x;
						break;
					case 1:
						vertices[i].BoneIndices[b] = (BYTE)fbxMeshVec[j]->m_OptVertexVec[k].m_BoneIndex.y;
						break;
					case 2:
						vertices[i].BoneIndices[b] = (BYTE)fbxMeshVec[j]->m_OptVertexVec[k].m_BoneIndex.z;
						break;
					case 3:
						vertices[i].BoneIndices[b] = (BYTE)fbxMeshVec[j]->m_OptVertexVec[k].m_BoneIndex.w;
						break;
					}
				}

				// 정규화하기
				EMath::Vector3 normalizedT = fbxMeshVec[j]->m_OptVertexVec[k].m_Tangent;
				normalizedT.Normalize();
				vertices[i].TangentU = normalizedT;

				// 버텍스의 최소, 최대 구하기
				EMath::Vector3 _pos = vertices[k].Pos;
				pObjectMeshData->m_MinVertexPos = EMath::Vector3::Min(pObjectMeshData->m_MinVertexPos, _pos);
				pObjectMeshData->m_MaxVertexPos = EMath::Vector3::Max(pObjectMeshData->m_MaxVertexPos, _pos);

				// 포지션 값 저장해놓음
				pObjectMeshData->m_VertexPosVec.push_back(_pos);
			}
		}

		// Index
		LoadIndices(pObjectMeshData, fbxMeshVec);
		CreateBuffers(pObjectMeshData, vertices);
	}
	break;
	}
}

void MeshBuilder::LoadIndices(ObjectMeshData* pObjectMeshData, std::vector<ParserData::Mesh*> fbxMeshVec)
{
	bool isFirst = false;
	size_t addIndex = 0;
	size_t startIndex = 0;
	for (size_t j = 0; j < fbxMeshVec.size(); j++)
	{
		size_t faceCount = fbxMeshVec[j]->m_FaceCount;

		pObjectMeshData->m_MeshVec[j]->StartIndex = startIndex;

		size_t indexCount = 3 * faceCount;
		startIndex += indexCount;
		pObjectMeshData->m_MeshVec[j]->IndexCount = indexCount;

		std::vector<UINT> meshIndices(indexCount);
		for (size_t i = 0; i < faceCount; ++i)
		{
			meshIndices[i * 3 + 0] = fbxMeshVec[j]->m_OptIndexVec[i * 3 + 0];
			meshIndices[i * 3 + 1] = fbxMeshVec[j]->m_OptIndexVec[i * 3 + 2];
			meshIndices[i * 3 + 2] = fbxMeshVec[j]->m_OptIndexVec[i * 3 + 1];
		}

		if (!isFirst)
		{
			pObjectMeshData->m_IndexVec.assign(meshIndices.begin(), meshIndices.end());
			isFirst = true;
		}
		else
		{
			addIndex += fbxMeshVec[j - 1]->m_OptVertexVec.size();
			size_t size = meshIndices.size();
			for (size_t i = 0; i < size; i++)
			{
				pObjectMeshData->m_IndexVec.push_back(meshIndices[i] + addIndex);
			}
		}
	}

	pObjectMeshData->m_IndexCount = pObjectMeshData->m_IndexVec.size();
}

void MeshBuilder::CreateBoundingBoxForPicking(std::wstring objName, ObjectMeshData* pObjectMeshData)
{
	EMath::Vector3 _min = pObjectMeshData->m_MinVertexPos;
	EMath::Vector3 _max = pObjectMeshData->m_MaxVertexPos;

	pObjectMeshData->m_BoundingBoxForPicking.Center = 0.5f * (_min + _max);
	pObjectMeshData->m_BoundingBoxForPicking.Extents = 0.5f * (_max - _min);

	// 지정한 컬러의 wireframe 박스 생성
	ObjectMeshInfo _BBInfo(ObjectMeshInfo::eMeshType::ColorBox, ObjectMeshInfo::eRenderStateType::Wire, 
		ObjectMeshInfo::eEffectType::Color, EMath::Vector4(0, 0, 1, 1), 
		pObjectMeshData->m_BoundingBoxForPicking.Extents,
		pObjectMeshData->m_BoundingBoxForPicking.Center);

	m_MyObjectMeshData = pObjectMeshData;
	CreateMeshes(objName + L"'s Pick Box", _BBInfo);
}

void MeshBuilder::MakeTangent(std::vector<ParserData::Mesh*> meshDataVec)
{
	/// 탄젠트 구하기
	for (UINT i = 0; i < meshDataVec.size(); i++)
	{
		// Face를 순회하면서
		for (UINT j = 0; j < meshDataVec[i]->m_FaceCount; j++)
		{
			// 인덱스
			UINT v0Index = meshDataVec[i]->m_OptIndexVec[j * 3 + 0];
			UINT v1Index = meshDataVec[i]->m_OptIndexVec[j * 3 + 1];
			UINT v2Index = meshDataVec[i]->m_OptIndexVec[j * 3 + 2];

			// 위 인덱스의 버텍스들의 Pos
			EMath::Vector3 v0Pos = meshDataVec[i]->m_OptVertexVec[v0Index].m_Pos;
			EMath::Vector3 v1Pos = meshDataVec[i]->m_OptVertexVec[v1Index].m_Pos;
			EMath::Vector3 v2Pos = meshDataVec[i]->m_OptVertexVec[v2Index].m_Pos;

			// 삼각형 면의 변 벡터
			EMath::Vector3 e1 = v1Pos - v0Pos;
			EMath::Vector3 e2 = v2Pos - v0Pos;

			// u
			float u0 = meshDataVec[i]->m_OptVertexVec[v0Index].m_UV.x;
			float u1 = meshDataVec[i]->m_OptVertexVec[v1Index].m_UV.x;
			float u2 = meshDataVec[i]->m_OptVertexVec[v2Index].m_UV.x;

			// v
			float v0 = meshDataVec[i]->m_OptVertexVec[v0Index].m_UV.y;
			float v1 = meshDataVec[i]->m_OptVertexVec[v1Index].m_UV.y;
			float v2 = meshDataVec[i]->m_OptVertexVec[v2Index].m_UV.y;

			// u, v 벡터
			float x1 = u1 - u0;
			float x2 = u2 - u0;
			float y1 = v1 - v0;
			float y2 = v2 - v0;

			// Tangent 구하기
			// 이용할 매트릭스들, uvMatrix의 역행렬을 바로넣었다.
			EMath::Matrix uvMatrixInverse(EMath::Vector3(y2, -x2, 0), EMath::Vector3(-y1, x1, 0), EMath::Vector3(0, 0, 0));
			EMath::Matrix e1e2Matrix(EMath::Vector3(e1.x, e2.x, 0), EMath::Vector3(e1.y, e2.y, 0), EMath::Vector3(e1.z, e2.z, 0));

			// 역행렬의 det
			float det = 1.0f / (x1 * y2 - x2 * y1);

			// tangent matrix, 탄젠트와 바이탄젠트가 들어있다. 바이탄젠트는 쉐이더에서 t와 n을 외적해 구하므로 저장하지않음
			EMath::Matrix tangentMatrix = det * e1e2Matrix * uvMatrixInverse;
			EMath::Vector3 t(tangentMatrix._11, tangentMatrix._21, tangentMatrix._31);

			// 페이스의 노말
			// 페이스를 이루고 있는 두 벡터의 외적
			// e1, e2를 외적하면 됨
			EMath::Vector3 n = e2.Cross(e1);
			t = t - (t.Dot(n)) * n;
			//t = (e1 * y2 - e2 * y1) * det;

			// tangent를 합산한다.
			meshDataVec[i]->m_OptVertexVec[v0Index].m_Tangent += t;
			meshDataVec[i]->m_OptVertexVec[v1Index].m_Tangent += t;
			meshDataVec[i]->m_OptVertexVec[v2Index].m_Tangent += t;
		}
	}
}
