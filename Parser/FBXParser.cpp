#include "FBXParser.h"
#include "MathConverter_Fbx.h"
#include "MeshAnimationData.h"
#include <iostream>

#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(m_pFBXManager->GetIOSettings()))
#endif

FBXParser::FBXParser()
	: m_NowMotion(nullptr), m_StartFrame(0), m_TotalFrame(0), m_TicksPerFrame(0), m_ParsingCount(0)
{
	EMath::Matrix _axisTransformMatrix
		(1, 0, 0, 0,
		0, 0, -1, 0,
		0, 1, 0, 0,
		0, 0, 0, 1);

	m_AxisTransformMatrix = _axisTransformMatrix;

	Initialize();
}

FBXParser::~FBXParser()
{
	m_pFBXManager->Destroy();
}

void FBXParser::Initialize()
{
	// FbxManager 클래스는 FBX SDK 용 메모리 관리자이다.
	// FBX SDK 객체를 인스턴스화 할 때마다 직접 또는 간접적으로 사용하고, 객체를 삭제할 때도 다시 사용합니다.

	// FBX 애플리케이션은 보통 아래와 같은 순설고 작성한다.
	// 1. SDK 관리자(FbxManager 클래스) 객체를 만들어서 시작
	// 2. 위 객체를 사용하여 씬(FbxScene 클래스) 객체를 만든다.
	// 3. 씬 객체를 사용하여 대부분의 FBX SDK 클래스의 객체를 만든다.

	// 일반적으로 FBX 애플리케이션은 하나의 SDK 관리자 객체만 있으면 된다. 
	// 일반적으로 이 FbxManager 싱글 톤의 생성은 모든 FBX SDK 프로그램에서 첫번째 행동이다.
	
	/// SDK 관리자 객체 생성
	m_pFBXManager = FbxManager::Create();
	if (!m_pFBXManager)
	{
		FBXSDK_printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", m_pFBXManager->GetVersion());

	/// I/O 설정 객체 만들기
	// FbxIOSettings 클래스는 씬의 요소를 파일에서 가져오거나 파일로 내보낼지 여부를 지정한다.
	// 이러한 요소에는 camera, light, mesh, texture, material, animation, 사용자 정의 속성 등이 포함된다.
	// FbxIOSettings 객체는 FbxImporter 또는 FbxExporter 객체에 전달되기 전에 인스턴스화되고 구성되어야한다.
	// FBX SDK의 대부분의 객체와 마찬가지로 FbxIOSettings 객체는 FbxManager 싱글 톤을 사용하여 생성되고 관리된다.
	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(m_pFBXManager, IOSROOT);
	m_pFBXManager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	m_pFBXManager->LoadPluginsDirectory(lPath.Buffer());
}

std::vector<ParserData::Mesh*> FBXParser::GetMeshVec(std::wstring fileName)
{
	if (m_FBXMeshVecUMap.count(fileName) > 0)
	{
		return m_FBXMeshVecUMap[fileName];
	}
	else
	{
		std::vector<ParserData::Mesh*> _fail;
		return _fail;
	}
}

std::unordered_map<std::wstring, Motion*> FBXParser::GetObjectMotionData(std::wstring name)
{
	if (m_ObjectMotionUMap.count(name) > 0)
	{
		return m_ObjectMotionUMap[name];
	}
	else
	{
		std::unordered_map<std::wstring, Motion*> _fail;
		return _fail;
	}
}

void FBXParser::LoadAllFBXFiles(bool isPrint)
{
	unsigned int _totalFilesCount = m_FilePathAndFileNameMap_FBX.size();
	m_ParsingCount = 0;

	// FBX 파일 경로를 순회하면서 하나씩 로드
	for (const auto& it : m_FilePathAndFileNameMap_FBX)
	{
		std::string _filePath;
		_filePath.assign(it.first.begin(), it.first.end());

		std::string _fileName;
		_fileName.assign(it.second.begin(), it.second.end());

		if (isPrint)
		{
			std::cout << "현재 " << _fileName << ".fbx 파싱 중...\n";
		}

		FbxScene* _result = LoadScene(_filePath.c_str());
		
		if (_result != nullptr)
		{
			ParseFbxScene(_result, it.second);
		}

		m_ParsingCount++;

		if (isPrint)
		{
			float _percent = (float)m_ParsingCount / _totalFilesCount * 100.0f;
			std::cout << _fileName << ".fbx 파싱 완료\n" << "진행률 : " << _percent << "%\n\n";
		}
	}
}

FbxScene* FBXParser::LoadScene(const char* fileName)
{
	/// 씬 객체 생성
	// 씬 객체를 만들 때 해당 씬의 메모리를 관리할 SDK 관리자 객체를 전달해야함
	// FBX 애플리케이션은 보통 하나의 씬을 필요로 한다. 
	// 그러나 만약 여러 개의 FBX 파일을 로드하고 동시에 사용하려는 경우에는 각 파일에 대한 씬을 생성해야 한다.
	// *주의* 씬 객체를 소멸하면 SDK 관리자가 자동으로 해당 씬 객체로 만든 모든 객체를 파괴한다.
	// Create an FBX scene. This object holds most objects imported/exported from/to files.
	FbxScene* _scene = FbxScene::Create(m_pFBXManager, fileName);
	if (!_scene)
	{
		return nullptr;
	}

	/// Global Scene Settings
	// 씬의 축 시스템, 주변 조명 및 시간 설정은 해당 FbxGlobalSettings 객체에 정의된다.
	// 이 객체는 FbxScene::GetGlobalSettings()를 통해 접근한다.

	/// FbxAxisSystem에 대해 링크 에러가 난다.
	/// 예상되는 문제 원인 : Parser 프로젝트에서 fbx의 모든 라이브러리를 추가 종속성으로 지정하지않았다. (현재 하나만 했음)
	
	bool lStatus;

	// Create an importer.
	FbxImporter* _fbxImporter = FbxImporter::Create(m_pFBXManager, "");

	/// Importer 초기화
	// 씬을 가져오는 기능은 FbxImporter 클래스에 의해 추상화된다. 
	// FbxImporter의 인스턴스는 FbxManager 싱글 톤 객체에 대한 참조로 만들어진다.
	// FbxImporter 객체는 반드시 세 개의 매개변수와 함께 FbxImporter::Initialize() 멤버 함수가 호출되어야 한다.
	// 여기서 첫 번째 매개변수는 가져오려는 fbx파일명이다.
	// Initialize the importer by providing a filename.
	const bool lImportStatus = _fbxImporter->Initialize(fileName, -1, m_pFBXManager->GetIOSettings());

	if (_fbxImporter->IsFBX())
	{
		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
		IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
		IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
		IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
		IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
		IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
		IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}

	/// FBX 파일 내용을 scene으로 가져온다.
	// Import the scene.
	lStatus = _fbxImporter->Import(_scene);

	/// 애니메이션이 있는 FBX인지 확인한다.
	FbxTakeInfo* _fbxTakeInfo = _fbxImporter->GetTakeInfo(0);
	
	// 애니메이션 데이터가 한 개라도 있을 경우
	if (_fbxTakeInfo != nullptr)
	{
		m_FbxTimeMode = _scene->GetGlobalSettings().GetTimeMode();
		float _frameRate = (float)FbxTime::GetFrameRate(m_FbxTimeMode);

		FbxTimeSpan _fbxTimeSpan = _fbxTakeInfo->mLocalTimeSpan;

		// 애니메이션의 시작시간과 종료시간
		double _tempStart = _fbxTimeSpan.GetStart().GetSecondDouble();
		double _tempStop = _fbxTimeSpan.GetStop().GetSecondDouble();

		// 시작시간보다 종료시간이 더 클 때 애니메이션이 있는 것이다.
		if (_tempStart < _tempStop)
		{
			// 구동시간 동안 총 몇 프레임이 수행될지를 keyFrame에 담는다.
			int _keyFrame = (int)((_tempStop - _tempStart) * (double)_frameRate);

			// 한 프레임의 재생 시간
			m_TicksPerFrame = (_tempStop - _tempStart) / _keyFrame;

			// 애니메이션의 시작과 끝 프레임
			m_StartFrame = (int)(_tempStart)*_keyFrame;
			int _endFrame = (int)(_tempStop)*_keyFrame;

			// 애니메이션의 총 프레임
			m_TotalFrame = _keyFrame;
		}
	}

	// Destroy the importer.
	_fbxImporter->Destroy();

	// 씬 내에서 삼각형화 할 수 있는 모든 노드를 삼각형화 시킨다.
	FbxGeometryConverter geometryConverter(m_pFBXManager);
	geometryConverter.Triangulate(_scene, true);
	return _scene;
}

void FBXParser::ParseFbxScene(FbxScene* scene, std::wstring fileName)
{
	FbxNode* _node = scene->GetRootNode();
	
	if (_node != nullptr)
	{
		for (size_t i = 0; i < _node->GetChildCount(); i++)
		{
			CreateMesh_And_LoadNodeData(scene, _node->GetChild(i));
		}
	}
	
	if (m_FBXMeshVec.size() > 0)
	{
		// 방금 파싱한 씬(FBX)을 맵에 저장함
		m_FBXMeshVecUMap[fileName] = m_FBXMeshVec;

		// 애니메이션이 있을 경우에
		// Skinned Mesh를 파싱하고
		// 모션(애니메이션 데이터들) 정리
		if (m_TotalFrame > 0
			&& m_pSkinnedMeshFbxNode != nullptr && m_pSkinnedMesh != nullptr)
		{
			// 본들의 파싱이 모두 끝났으므로 Skinned Mesh의 파싱 진행
			LoadSkinnedMeshData(scene);

			// 첫번째 노드의 이름 저장해놓음 (오브젝트 구분용)
			std::string _firstNodeName = m_FBXMeshVec[0]->m_NodeName;

			m_NowMotion->m_ObjectNodeName = _firstNodeName;

			// 모션의 이름
			std::wstring _motionName;

			// 파일 이름의 끝에서 _가 몇번째인지 찾는다.
			size_t findIndex = fileName.find_last_of(L"_");

			// _가 없다면 Default 이름으로 설정
			if (findIndex == -1)
			{
				_motionName = L"None";
			}
			else
			{
				findIndex++;

				// 모션 이름과 확장자만 남김
				_motionName = fileName.substr(findIndex, fileName.size() - findIndex);

				// 확장자 제거
				_motionName = _motionName.substr(0, _motionName.size() - 4);
			}

			// 모션의 이름을 저장
			m_NowMotion->m_MotionName.assign(_motionName.begin(), _motionName.end());

			/// 같은 오브젝트인지 체크해야함
			std::unordered_map<std::wstring, Motion*> _motionMap = m_ObjectMotionUMap[fileName];

			// 같은 이름의 오브젝트가 맵에 없다면
			if (_motionMap.size() <= 0)
			{
				_motionMap[_motionName] = m_NowMotion.get();
				m_ObjectMotionUMap[fileName] = _motionMap;
			}
			// 이미 있다면 기존 벡터에 넣는다. (같은 오브젝트이므로)
			else
			{
				m_ObjectMotionUMap[fileName][_motionName] = m_NowMotion.get();
			}

			m_UniqueMotionVec.push_back(std::move(m_NowMotion));
		}
;	}

	/// 초기화 /////////////
	// 메쉬 /////
	// 현재 파싱하고 있는 메쉬 벡터는 초기화
	m_FBXMeshVec.clear();

	// 스플릿을 위한 맵도 초기화
	m_CheckSplitedVertexMap.clear();

	// 애니메이션 /////
	m_NowMotion = nullptr;
	m_NowBoneIndex = 0;

	m_StartFrame = 0;
	m_TotalFrame = 0;
	m_TicksPerFrame = 0;

	m_pSkinnedMeshFbxNode = nullptr;
	m_pSkinnedMesh = nullptr;
	m_BoneVec.clear();
}

void FBXParser::CreateMesh_And_LoadNodeData(FbxScene* scene, FbxNode* node)
{
	FbxNodeAttribute* _fbxNodeAttribute = node->GetNodeAttribute();

	if (_fbxNodeAttribute != nullptr)
	{
		FbxNodeAttribute::EType _type = _fbxNodeAttribute->GetAttributeType();

		// 노드가 메쉬, 본일 경우만 메쉬를 생성, (Camera나 Light는 건너뛴다.)
		if (_type == FbxNodeAttribute::eMesh
			|| _type == FbxNodeAttribute::eSkeleton)
		{
			std::unique_ptr<ParserData::Mesh> _mesh = std::make_unique<ParserData::Mesh>();

			// 일단, 노드 = 메쉬라고 가정
			// 현재 노드의 이름
			_mesh->m_NodeName = node->GetName();

			// 부모가 있다면
			if (node->GetParent() != nullptr)
			{
				_mesh->m_NodeParentName = node->GetParent()->GetName();
			}

			/// Negative Scale
			//CheckNegativeScale(_mesh);

			// Bone이 아닌 메쉬일 경우
			if (_type == FbxNodeAttribute::eMesh)
			{
				FbxMesh* _fbxMesh = node->GetMesh();

				/// 애니메이션이 있는지 검사
				int _deformerCount = _fbxMesh->GetDeformerCount();

				// Deformer(Bone)의 개수가 1개 이상이면 이 Fbx는 애니메이션 데이터가 있고, 현재 메쉬는 SkinnedMesh이다.
				if (_deformerCount > 0)
				{
					// 애니메이션 데이터가 있으므로 모션을 생성해준다.
					if (m_NowMotion == nullptr)
					{
						m_NowMotion = std::make_unique<Motion>();
					}

					// 한 프레임의 재생 시간을 넣어줌
					m_NowMotion->m_TicksPerFrame = static_cast<float>(m_TicksPerFrame);

					// 애니메이션이 있다면 Skinned Mesh만 체크해주고
					// 본을 모두 읽은 후에 Skinned Mesh 파싱을 진행한다.
					_mesh->m_IsSkinnedMesh = true;
					
					m_pSkinnedMeshFbxNode = node;
					m_pSkinnedMesh = _mesh.get();
				}
				else
				{
					/// Pos
					CreateVertex_And_LoadVertexPos(node, _mesh.get());	// 이 함수를 지나면 FBXParserData::Mesh의 Vertex는 모두 생성되어있음

					/// Normal, UV, Tangent...
					LoadVertexOthers(node, _mesh.get());				// 여기서 Face를 순회하면서 Vertex의 나머지 정보들을 넣고, Index도 넣어줌
				}
			}
			// Bone일 경우
			else if (_type == FbxNodeAttribute::eSkeleton)
			{
				m_BoneVec.push_back(_mesh.get());

				LoadAnimationData(scene, node);
			}

			// WorldTM, LocalTM
			FbxMatrix worldTM = scene->GetAnimationEvaluator()->GetNodeGlobalTransform(node);
			FbxMatrix localTM = scene->GetAnimationEvaluator()->GetNodeLocalTransform(node);

			_mesh->m_NodeTM = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(worldTM, true);
			_mesh->m_LocalTM = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(localTM, true);

			_mesh->m_NodeInverseTM = _mesh->m_NodeTM.Invert();

			m_FBXMeshVec.push_back(_mesh.get());
			m_UniqueMeshVec.push_back(std::move(_mesh));
		}
	}

	for (size_t i = 0; i < node->GetChildCount(); i++)
	{
		CreateMesh_And_LoadNodeData(scene, node->GetChild(i));
	}
}

void FBXParser::CreateVertex_And_LoadVertexPos(FbxNode* node, ParserData::Mesh* mesh)
{
	FbxMesh* _fbxMesh = node->GetMesh();

	unsigned int _controlPointCount = _fbxMesh->GetControlPointsCount();

	for (unsigned int i = 0; i < _controlPointCount; i++)
	{
		ParserData::Vertex _vertex;

		/// Pos의 y,z 뒤집어줌
		_vertex.m_Pos.x = -static_cast<float>(_fbxMesh->GetControlPointAt(i).mData[0]);
		_vertex.m_Pos.y = static_cast<float>(_fbxMesh->GetControlPointAt(i).mData[1]);
		_vertex.m_Pos.z = static_cast<float>(_fbxMesh->GetControlPointAt(i).mData[2]);

		_vertex.m_Index = i;

		mesh->m_VertexVec.push_back(_vertex);
	}
}

void FBXParser::CreateVertex_And_LoadVertexPos_Animation(FbxNode* node, ParserData::Mesh* mesh)
{
	FbxMesh* _fbxMesh = node->GetMesh();

	unsigned int _controlPointCount = _fbxMesh->GetControlPointsCount();

	for (unsigned int i = 0; i < _controlPointCount; i++)
	{
		ParserData::Vertex _vertex;

		/// Pos의 y,z 뒤집어줌
		_vertex.m_Pos.x = static_cast<float>(_fbxMesh->GetControlPointAt(i).mData[0]);
		_vertex.m_Pos.y = static_cast<float>(_fbxMesh->GetControlPointAt(i).mData[2]);
		_vertex.m_Pos.z = static_cast<float>(_fbxMesh->GetControlPointAt(i).mData[1]);

		_vertex.m_Index = i;

		mesh->m_VertexVec.push_back(_vertex);
	}
}

void FBXParser::LoadVertexOthers(FbxNode* node, ParserData::Mesh* mesh)
{
	FbxMesh* _fbxMesh = node->GetMesh();
	mesh->m_FaceCount = _fbxMesh->GetPolygonCount();
	unsigned int _vertexCount = 0;

	for (unsigned int i = 0; i < mesh->m_FaceCount; i++)
	{
		for (unsigned int j = 0; j < 3; j++)
		{
			int _vertexIndex = _fbxMesh->GetPolygonVertex(i, j);

			// Normal
			if (_fbxMesh->GetElementNormalCount() > 0)
			{
				mesh->m_VertexVec[_vertexIndex].m_Normal = ReadNormal(_fbxMesh, _vertexIndex, _vertexCount);

				// Negative Scale일 경우 Normal을 뒤집어준다.
				if (mesh->m_IsNegativeScale)
				{
					mesh->m_VertexVec[_vertexIndex].m_Normal *= -1;
				}
			}

			// UV
			if (_fbxMesh->GetElementUVCount() > 0)
			{
				mesh->m_VertexVec[_vertexIndex].m_UV = ReadUV(_fbxMesh, _vertexIndex, _vertexCount);
			}
			
			SplitVertex(mesh, mesh->m_VertexVec[_vertexIndex]);

			_vertexCount++;
		}	
	}
}

void FBXParser::SplitVertex(ParserData::Mesh* mesh, ParserData::Vertex& vertex)
{
	/*
	auto findVertex = m_CheckSplitedVertexMap.find(vertex);
	
	if (findVertex != m_CheckSplitedVertexMap.end())
	{
		// map에 vertex가 이미 존재한다.
		mesh->m_OptIndexVec.push_back(findVertex->second);
	}
	else
	{
		// map에 vertex가 존재하지 않는다.
		// 존재하지 않으면 새로운 정점을 만들고, index를 지정
		size_t _vertexIndex = m_CheckSplitedVertexMap.size();
		mesh->m_OptIndexVec.push_back(_vertexIndex);
		
		mesh->m_OptVertexVec.push_back(vertex);
		m_CheckSplitedVertexMap[vertex] = _vertexIndex;
	}
	*/
	/// 스플릿 안하려면
	mesh->m_OptIndexVec.push_back(mesh->m_OptVertexVec.size());
	mesh->m_OptVertexVec.push_back(vertex);
}

void FBXParser::CheckNegativeScale(ParserData::Mesh* mesh)
{
	// [ Negative Scale 확인 방법 ]
	// row0, row1, row2가 각각 x,y,z축을 나타내므로 이 중 두 축을 '외적'한 값이 다른 한 축과 방향이 반대인지 혹은 같은지를 확인하면된다.
	// -> 외적 한 값과 다른 한 축을 내적한다. (음수일 경우 축이 반대, 양수일 경우 축이 같은 방향)
	EMath::Vector3 _row0(mesh->m_NodeTM._11, mesh->m_NodeTM._12, mesh->m_NodeTM._13);
	EMath::Vector3 _row1(mesh->m_NodeTM._21, mesh->m_NodeTM._22, mesh->m_NodeTM._23);
	EMath::Vector3 _row2(mesh->m_NodeTM._31, mesh->m_NodeTM._32, mesh->m_NodeTM._33);

	EMath::Vector3 _crossResult = _row0.Cross(_row1);
	float checkNegativeScaleResult = _crossResult.Dot(_row2);

	// 음수면 축이 반대이다.
	if (checkNegativeScaleResult < 0)
	{
		mesh->m_IsNegativeScale = true;

		// NodeTM
		EMath::Vector3 _scale;
		EMath::Quaternion _rot;
		EMath::Vector3 _trans;
		mesh->m_NodeTM.Decompose(_scale, _rot, _trans);

		// 스케일을 뒤집어줌
		if (_scale.x < 0)
		{
			_scale.x *= -1;
		}
		if (_scale.y < 0)
		{
			_scale.y *= -1;
		}
		if (_scale.z < 0)
		{
			_scale.z *= -1;
		}

		EMath::Matrix _scaleTM = EMath::Matrix::CreateScale(_scale);
		EMath::Matrix _rotTM = EMath::Matrix::CreateFromQuaternion(_rot);
		EMath::Matrix _transTM = EMath::Matrix::CreateTranslation(_trans);

		mesh->m_NodeTM = _scaleTM * _rotTM * _transTM;

		// LocalTM
		mesh->m_LocalTM.Decompose(_scale, _rot, _trans);
		
		// 스케일을 뒤집어줌
		if (_scale.x < 0)
		{
			_scale.x *= -1;
		}
		if (_scale.y < 0)
		{
			_scale.y *= -1;
		}
		if (_scale.z < 0)
		{
			_scale.z *= -1;
		}

		_scaleTM = EMath::Matrix::CreateScale(_scale);
		_rotTM = EMath::Matrix::CreateFromQuaternion(_rot);
		_transTM = EMath::Matrix::CreateTranslation(_trans);

		mesh->m_LocalTM = _scaleTM * _rotTM * _transTM;
	}
}

void FBXParser::DownScale(ParserData::Mesh* mesh)
{
	// FBX에서는 mm단위로 내보낸다. 즉, 1m를 내보내면 1000mm???
	// 아무튼 유니티에서는 블렌더에서 스케일 100이 유니티에서 스케일 1과 같다. (Down Scale의 이유)

	/// World
	EMath::Vector3 _scale;
	EMath::Quaternion _rot;
	EMath::Vector3 _trans;
	mesh->m_NodeTM.Decompose(_scale, _rot, _trans);

	// 스케일 축소
	_scale *= 0.01f;

	EMath::Matrix _scaleTM = EMath::Matrix::CreateScale(_scale);
	EMath::Matrix _rotTM = EMath::Matrix::CreateFromQuaternion(_rot);
	EMath::Matrix _transTM = EMath::Matrix::CreateTranslation(_trans);

	mesh->m_NodeTM = _scaleTM * _rotTM * _transTM;

	/// Local
	mesh->m_LocalTM.Decompose(_scale, _rot, _trans);

	// 스케일 축소
	_scale *= 0.01f;

	_scaleTM = EMath::Matrix::CreateScale(_scale);
	_rotTM = EMath::Matrix::CreateFromQuaternion(_rot);
	_transTM = EMath::Matrix::CreateTranslation(_trans);

	mesh->m_LocalTM = _scaleTM * _rotTM * _transTM;
}

void FBXParser::LoadSkinnedMeshData(FbxScene* scene)
{
	// Skinned Mesh에 대한 파싱 진행

	/// Pos
	CreateVertex_And_LoadVertexPos_Animation(m_pSkinnedMeshFbxNode, m_pSkinnedMesh);		// 이 함수를 지나면 FBXParserData::Mesh의 Vertex는 모두 생성되어있음

	/// Weights
	LoadBoneWeights(m_pSkinnedMeshFbxNode, m_pSkinnedMesh);

	/// Normal, UV, Tangent...
	LoadVertexOthers(m_pSkinnedMeshFbxNode, m_pSkinnedMesh);					// 여기서 Face를 순회하면서 Vertex의 나머지 정보들을 넣고, Index도 넣어줌
}

void FBXParser::LoadBoneWeights(FbxNode* node, ParserData::Mesh* mesh)
{
	FbxMesh* _fbxMesh = node->GetMesh();
	int _deformerCount = _fbxMesh->GetDeformerCount();

	for (size_t i = 0; i < _deformerCount; i++)
	{
		FbxDeformer* _fbxDeformer = _fbxMesh->GetDeformer(i);

		if (_fbxDeformer != nullptr && _fbxDeformer->GetDeformerType() == FbxDeformer::eSkin)
		{
			FbxSkin* _fbxSkin = reinterpret_cast<FbxSkin*>(_fbxMesh->GetDeformer(i, FbxDeformer::eSkin));

			if (_fbxSkin != nullptr)
			{
				FbxCluster::ELinkMode _fbxClusterLinkMode = FbxCluster::eNormalize;

				unsigned int _clusterCount = _fbxSkin->GetClusterCount();

				for (unsigned int boneIndex = 0; boneIndex < _clusterCount; boneIndex++)
				{
					FbxCluster* _fbxCluster = _fbxSkin->GetCluster(boneIndex);

					_fbxClusterLinkMode = _fbxCluster->GetLinkMode();

					FbxNode* _linkFbxNode = _fbxCluster->GetLink();
					std::string _linkFbxNodeName = _linkFbxNode->GetName();

					FbxMesh* __linkFbxMesh = _linkFbxNode->GetMesh();

					for (unsigned int k = 0; k < m_BoneVec.size(); k++)
					{
						if (m_BoneVec[k]->m_NodeName == _linkFbxNodeName)
						{
							m_BoneVec[k]->m_BoneIndex = boneIndex;
							m_BoneVec[k]->m_IsBone = true;
							break;
						}
					}

					unsigned int _controlPointIndicesCount = _fbxCluster->GetControlPointIndicesCount();
					for (unsigned int c = 0; c < _controlPointIndicesCount; c++)
					{
						int _controlPointIndex = _fbxCluster->GetControlPointIndices()[c];
						float _weight = static_cast<float>(_fbxCluster->GetControlPointWeights()[c]);

						if (_weight <= 0)
							continue;

						switch (mesh->m_VertexVec[_controlPointIndex].m_BoneSize)
						{
						case 0:
							mesh->m_VertexVec[_controlPointIndex].m_BoneWeight.x = _weight;
							mesh->m_VertexVec[_controlPointIndex].m_BoneIndex.x = boneIndex;
							break;
						case 1:
							mesh->m_VertexVec[_controlPointIndex].m_BoneWeight.y = _weight;
							mesh->m_VertexVec[_controlPointIndex].m_BoneIndex.y = boneIndex;
							break;
						case 2:
							mesh->m_VertexVec[_controlPointIndex].m_BoneWeight.z = _weight;
							mesh->m_VertexVec[_controlPointIndex].m_BoneIndex.z = boneIndex;
							break;
						case 3:
							mesh->m_VertexVec[_controlPointIndex].m_BoneIndex.w = boneIndex;
							break;
						}

						mesh->m_VertexVec[_controlPointIndex].m_BoneSize++;
					}

					FbxAMatrix _matClusterTransformMatrix;
					FbxAMatrix _matClusterLinkTransformMatrix;

					const FbxVector4 lT = node->GetGeometricTranslation(FbxNode::eDestinationPivot);
					const FbxVector4 lR = node->GetGeometricRotation(FbxNode::eDestinationPivot);
					const FbxVector4 lS = node->GetGeometricScaling(FbxNode::eDestinationPivot);

					FbxAMatrix _geometryTransform = FbxAMatrix(lT, lR, lS);

					_fbxCluster->GetTransformMatrix(_matClusterTransformMatrix);
					_fbxCluster->GetTransformLinkMatrix(_matClusterLinkTransformMatrix);

					// Bone Matrix 설정
					EMath::Matrix clusterMatrix = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(_matClusterTransformMatrix);
					EMath::Matrix clusterlinkMatrix = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(_matClusterLinkTransformMatrix);
					EMath::Matrix geometryMatrix = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(_geometryTransform);

					EMath::Matrix offsetMatrix = clusterlinkMatrix.Invert() * clusterMatrix *  geometryMatrix;

					//m_pNowMotion->m_BoneOffsetTransformVec[boneIndex] = m_BoneVec[boneIndex]->m_NodeTM;
					m_NowMotion->m_BoneOffsetName.push_back(_linkFbxNodeName);
				}

				// 가중치 1로?
				// ...
				// ...
			}
		}
	}
}

void FBXParser::LoadAnimationData(FbxScene* scene, FbxNode* node)
{
	/// 본으로부터 애니메이션 데이터를 로드한다.
	// 애니메이션 데이터 생성
	std::unique_ptr<MeshAnimationData> _meshAnimationData = std::make_unique<MeshAnimationData>();

	// 나중에 노드 네임으로 메쉬를 연결시키기 위해 이름을 저장
	_meshAnimationData->m_NodeName = node->GetName();

	for (FbxLongLong i = 0; i < m_TotalFrame; i++)
	{
		FbxTime _takeTime;
		_takeTime.SetFrame(m_StartFrame + i, m_FbxTimeMode);

		// 현재 프레임의 로컬 트랜스폼
		FbxAMatrix _animFbxTM = node->EvaluateLocalTransform(_takeTime);

		EMath::Matrix _animTM = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(_animFbxTM, true);

		EMath::Vector3 _scale;
		EMath::Quaternion _rot;
		EMath::Vector3 _trans;

		_animTM.Decompose(_scale, _rot, _trans);

		// Pos 키프레임, Pos Sample 넣기
		//_meshAnimationData->m_PosKeyFrameVec.push_back(i);
		//_meshAnimationData->m_PosSampleVec.push_back(_trans);

		// Rotation 키프레임, Rot Sample 넣기
		//_meshAnimationData->m_RotKeyFrameVec.push_back(i);
		//_meshAnimationData->m_RotSampleVec.push_back(_rot);

		// Local Transform을 바로 넣기
		_meshAnimationData->m_AnimationTMVec.push_back(_animTM);
	}

	// 이 본의 애니메이션 데이터를 모션에 넣는다.
	if (m_NowMotion != nullptr)
	{
		m_NowMotion->m_AnimationDataVec.push_back(_meshAnimationData.get());
		m_UniqueAnimDataVec.push_back(std::move(_meshAnimationData));
	}
}

EMath::Vector3 FBXParser::ReadNormal(FbxMesh* fbxMesh, int vertexIndex, int vertexCount)
{
	EMath::Vector3 _result;
	FbxGeometryElementNormal* _vertexNormal = fbxMesh->GetElementNormal(0);

	switch (_vertexNormal->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (_vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			_result.x = -static_cast<float>(_vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[0]);
			_result.y = static_cast<float>(_vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[1]);
			_result.z = static_cast<float>(_vertexNormal->GetDirectArray().GetAt(vertexIndex).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int _index = _vertexNormal->GetIndexArray().GetAt(vertexIndex);
			_result.x = -static_cast<float>(_vertexNormal->GetDirectArray().GetAt(_index).mData[0]);
			_result.y = static_cast<float>(_vertexNormal->GetDirectArray().GetAt(_index).mData[1]);
			_result.z = static_cast<float>(_vertexNormal->GetDirectArray().GetAt(_index).mData[2]);
		}
		break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (_vertexNormal->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			_result.x = -static_cast<float>(_vertexNormal->GetDirectArray().GetAt(vertexCount).mData[0]);
			_result.y = static_cast<float>(_vertexNormal->GetDirectArray().GetAt(vertexCount).mData[1]);
			_result.z = static_cast<float>(_vertexNormal->GetDirectArray().GetAt(vertexCount).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int _index = _vertexNormal->GetIndexArray().GetAt(vertexCount);
			_result.x = -static_cast<float>(_vertexNormal->GetDirectArray().GetAt(_index).mData[0]);
			_result.y = static_cast<float>(_vertexNormal->GetDirectArray().GetAt(_index).mData[1]);
			_result.z = static_cast<float>(_vertexNormal->GetDirectArray().GetAt(_index).mData[2]);
		}
		break;
		}
	}
	break;
	}

	return _result;
}

EMath::Vector2 FBXParser::ReadUV(FbxMesh* fbxMesh, int vertexIndex, int vertexCount)
{
	EMath::Vector2 _result;
	FbxGeometryElementUV* _vertexUV = fbxMesh->GetElementUV();
	
	switch (_vertexUV->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (_vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			_result.x = static_cast<float>(_vertexUV->GetDirectArray().GetAt(vertexIndex).mData[0]);
			_result.y = static_cast<float>(1.0f - _vertexUV->GetDirectArray().GetAt(vertexIndex).mData[1]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int _index = _vertexUV->GetIndexArray().GetAt(vertexIndex);
			_result.x = static_cast<float>(_vertexUV->GetDirectArray().GetAt(_index).mData[0]);
			_result.y = static_cast<float>(1.0f - _vertexUV->GetDirectArray().GetAt(_index).mData[1]);
		}
		break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (_vertexUV->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			_result.x = static_cast<float>(_vertexUV->GetDirectArray().GetAt(vertexCount).mData[0]);
			_result.y = static_cast<float>(1.0f - _vertexUV->GetDirectArray().GetAt(vertexCount).mData[1]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int _index = _vertexUV->GetIndexArray().GetAt(vertexCount);
			_result.x = static_cast<float>(_vertexUV->GetDirectArray().GetAt(_index).mData[0]);
			_result.y = static_cast<float>(1.0f - _vertexUV->GetDirectArray().GetAt(_index).mData[1]);
		}
		break;
		}
	}
	break;
	}

	return _result;
}

EMath::Vector3 FBXParser::ReadTangent(FbxMesh* fbxMesh, int vertexIndex, int vertexCount)
{
	EMath::Vector3 _result;
	FbxGeometryElementTangent* _vertexTangent = fbxMesh->GetElementTangent(0);

	switch (_vertexTangent->GetMappingMode())
	{
	case FbxGeometryElement::eByControlPoint:
	{
		switch (_vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			_result.x = -static_cast<float>(_vertexTangent->GetDirectArray().GetAt(vertexIndex).mData[0]);
			_result.y = static_cast<float>(_vertexTangent->GetDirectArray().GetAt(vertexIndex).mData[1]);
			_result.z = static_cast<float>(_vertexTangent->GetDirectArray().GetAt(vertexIndex).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int _index = _vertexTangent->GetIndexArray().GetAt(vertexIndex);
			_result.x = -static_cast<float>(_vertexTangent->GetDirectArray().GetAt(_index).mData[0]);
			_result.y = static_cast<float>(_vertexTangent->GetDirectArray().GetAt(_index).mData[1]);
			_result.z = static_cast<float>(_vertexTangent->GetDirectArray().GetAt(_index).mData[2]);
		}
		break;
		}
	}
	break;

	case FbxGeometryElement::eByPolygonVertex:
	{
		switch (_vertexTangent->GetReferenceMode())
		{
		case FbxGeometryElement::eDirect:
		{
			_result.x = -static_cast<float>(_vertexTangent->GetDirectArray().GetAt(vertexCount).mData[0]);
			_result.y = static_cast<float>(_vertexTangent->GetDirectArray().GetAt(vertexCount).mData[1]);
			_result.z = static_cast<float>(_vertexTangent->GetDirectArray().GetAt(vertexCount).mData[2]);
		}
		break;
		case FbxGeometryElement::eIndexToDirect:
		{
			int _index = _vertexTangent->GetIndexArray().GetAt(vertexCount);
			_result.x = -static_cast<float>(_vertexTangent->GetDirectArray().GetAt(_index).mData[0]);
			_result.y = static_cast<float>(_vertexTangent->GetDirectArray().GetAt(_index).mData[1]);
			_result.z = static_cast<float>(_vertexTangent->GetDirectArray().GetAt(_index).mData[2]);
		}
		break;
		}
	}
	break;
	}

	return _result;
}
