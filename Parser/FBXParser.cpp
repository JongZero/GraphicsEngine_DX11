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
	// FbxManager Ŭ������ FBX SDK �� �޸� �������̴�.
	// FBX SDK ��ü�� �ν��Ͻ�ȭ �� ������ ���� �Ǵ� ���������� ����ϰ�, ��ü�� ������ ���� �ٽ� ����մϴ�.

	// FBX ���ø����̼��� ���� �Ʒ��� ���� ������ �ۼ��Ѵ�.
	// 1. SDK ������(FbxManager Ŭ����) ��ü�� ���� ����
	// 2. �� ��ü�� ����Ͽ� ��(FbxScene Ŭ����) ��ü�� �����.
	// 3. �� ��ü�� ����Ͽ� ��κ��� FBX SDK Ŭ������ ��ü�� �����.

	// �Ϲ������� FBX ���ø����̼��� �ϳ��� SDK ������ ��ü�� ������ �ȴ�. 
	// �Ϲ������� �� FbxManager �̱� ���� ������ ��� FBX SDK ���α׷����� ù��° �ൿ�̴�.
	
	/// SDK ������ ��ü ����
	m_pFBXManager = FbxManager::Create();
	if (!m_pFBXManager)
	{
		FBXSDK_printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", m_pFBXManager->GetVersion());

	/// I/O ���� ��ü �����
	// FbxIOSettings Ŭ������ ���� ��Ҹ� ���Ͽ��� �������ų� ���Ϸ� �������� ���θ� �����Ѵ�.
	// �̷��� ��ҿ��� camera, light, mesh, texture, material, animation, ����� ���� �Ӽ� ���� ���Եȴ�.
	// FbxIOSettings ��ü�� FbxImporter �Ǵ� FbxExporter ��ü�� ���޵Ǳ� ���� �ν��Ͻ�ȭ�ǰ� �����Ǿ���Ѵ�.
	// FBX SDK�� ��κ��� ��ü�� ���������� FbxIOSettings ��ü�� FbxManager �̱� ���� ����Ͽ� �����ǰ� �����ȴ�.
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

	// FBX ���� ��θ� ��ȸ�ϸ鼭 �ϳ��� �ε�
	for (const auto& it : m_FilePathAndFileNameMap_FBX)
	{
		std::string _filePath;
		_filePath.assign(it.first.begin(), it.first.end());

		std::string _fileName;
		_fileName.assign(it.second.begin(), it.second.end());

		if (isPrint)
		{
			std::cout << "���� " << _fileName << ".fbx �Ľ� ��...\n";
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
			std::cout << _fileName << ".fbx �Ľ� �Ϸ�\n" << "����� : " << _percent << "%\n\n";
		}
	}
}

FbxScene* FBXParser::LoadScene(const char* fileName)
{
	/// �� ��ü ����
	// �� ��ü�� ���� �� �ش� ���� �޸𸮸� ������ SDK ������ ��ü�� �����ؾ���
	// FBX ���ø����̼��� ���� �ϳ��� ���� �ʿ�� �Ѵ�. 
	// �׷��� ���� ���� ���� FBX ������ �ε��ϰ� ���ÿ� ����Ϸ��� ��쿡�� �� ���Ͽ� ���� ���� �����ؾ� �Ѵ�.
	// *����* �� ��ü�� �Ҹ��ϸ� SDK �����ڰ� �ڵ����� �ش� �� ��ü�� ���� ��� ��ü�� �ı��Ѵ�.
	// Create an FBX scene. This object holds most objects imported/exported from/to files.
	FbxScene* _scene = FbxScene::Create(m_pFBXManager, fileName);
	if (!_scene)
	{
		return nullptr;
	}

	/// Global Scene Settings
	// ���� �� �ý���, �ֺ� ���� �� �ð� ������ �ش� FbxGlobalSettings ��ü�� ���ǵȴ�.
	// �� ��ü�� FbxScene::GetGlobalSettings()�� ���� �����Ѵ�.

	/// FbxAxisSystem�� ���� ��ũ ������ ����.
	/// ����Ǵ� ���� ���� : Parser ������Ʈ���� fbx�� ��� ���̺귯���� �߰� ���Ӽ����� ���������ʾҴ�. (���� �ϳ��� ����)
	
	bool lStatus;

	// Create an importer.
	FbxImporter* _fbxImporter = FbxImporter::Create(m_pFBXManager, "");

	/// Importer �ʱ�ȭ
	// ���� �������� ����� FbxImporter Ŭ������ ���� �߻�ȭ�ȴ�. 
	// FbxImporter�� �ν��Ͻ��� FbxManager �̱� �� ��ü�� ���� ������ ���������.
	// FbxImporter ��ü�� �ݵ�� �� ���� �Ű������� �Բ� FbxImporter::Initialize() ��� �Լ��� ȣ��Ǿ�� �Ѵ�.
	// ���⼭ ù ��° �Ű������� ���������� fbx���ϸ��̴�.
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

	/// FBX ���� ������ scene���� �����´�.
	// Import the scene.
	lStatus = _fbxImporter->Import(_scene);

	/// �ִϸ��̼��� �ִ� FBX���� Ȯ���Ѵ�.
	FbxTakeInfo* _fbxTakeInfo = _fbxImporter->GetTakeInfo(0);
	
	// �ִϸ��̼� �����Ͱ� �� ���� ���� ���
	if (_fbxTakeInfo != nullptr)
	{
		m_FbxTimeMode = _scene->GetGlobalSettings().GetTimeMode();
		float _frameRate = (float)FbxTime::GetFrameRate(m_FbxTimeMode);

		FbxTimeSpan _fbxTimeSpan = _fbxTakeInfo->mLocalTimeSpan;

		// �ִϸ��̼��� ���۽ð��� ����ð�
		double _tempStart = _fbxTimeSpan.GetStart().GetSecondDouble();
		double _tempStop = _fbxTimeSpan.GetStop().GetSecondDouble();

		// ���۽ð����� ����ð��� �� Ŭ �� �ִϸ��̼��� �ִ� ���̴�.
		if (_tempStart < _tempStop)
		{
			// �����ð� ���� �� �� �������� ��������� keyFrame�� ��´�.
			int _keyFrame = (int)((_tempStop - _tempStart) * (double)_frameRate);

			// �� �������� ��� �ð�
			m_TicksPerFrame = (_tempStop - _tempStart) / _keyFrame;

			// �ִϸ��̼��� ���۰� �� ������
			m_StartFrame = (int)(_tempStart)*_keyFrame;
			int _endFrame = (int)(_tempStop)*_keyFrame;

			// �ִϸ��̼��� �� ������
			m_TotalFrame = _keyFrame;
		}
	}

	// Destroy the importer.
	_fbxImporter->Destroy();

	// �� ������ �ﰢ��ȭ �� �� �ִ� ��� ��带 �ﰢ��ȭ ��Ų��.
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
		// ��� �Ľ��� ��(FBX)�� �ʿ� ������
		m_FBXMeshVecUMap[fileName] = m_FBXMeshVec;

		// �ִϸ��̼��� ���� ��쿡
		// Skinned Mesh�� �Ľ��ϰ�
		// ���(�ִϸ��̼� �����͵�) ����
		if (m_TotalFrame > 0
			&& m_pSkinnedMeshFbxNode != nullptr && m_pSkinnedMesh != nullptr)
		{
			// ������ �Ľ��� ��� �������Ƿ� Skinned Mesh�� �Ľ� ����
			LoadSkinnedMeshData(scene);

			// ù��° ����� �̸� �����س��� (������Ʈ ���п�)
			std::string _firstNodeName = m_FBXMeshVec[0]->m_NodeName;

			m_NowMotion->m_ObjectNodeName = _firstNodeName;

			// ����� �̸�
			std::wstring _motionName;

			// ���� �̸��� ������ _�� ���°���� ã�´�.
			size_t findIndex = fileName.find_last_of(L"_");

			// _�� ���ٸ� Default �̸����� ����
			if (findIndex == -1)
			{
				_motionName = L"None";
			}
			else
			{
				findIndex++;

				// ��� �̸��� Ȯ���ڸ� ����
				_motionName = fileName.substr(findIndex, fileName.size() - findIndex);

				// Ȯ���� ����
				_motionName = _motionName.substr(0, _motionName.size() - 4);
			}

			// ����� �̸��� ����
			m_NowMotion->m_MotionName.assign(_motionName.begin(), _motionName.end());

			/// ���� ������Ʈ���� üũ�ؾ���
			std::unordered_map<std::wstring, Motion*> _motionMap = m_ObjectMotionUMap[fileName];

			// ���� �̸��� ������Ʈ�� �ʿ� ���ٸ�
			if (_motionMap.size() <= 0)
			{
				_motionMap[_motionName] = m_NowMotion.get();
				m_ObjectMotionUMap[fileName] = _motionMap;
			}
			// �̹� �ִٸ� ���� ���Ϳ� �ִ´�. (���� ������Ʈ�̹Ƿ�)
			else
			{
				m_ObjectMotionUMap[fileName][_motionName] = m_NowMotion.get();
			}

			m_UniqueMotionVec.push_back(std::move(m_NowMotion));
		}
;	}

	/// �ʱ�ȭ /////////////
	// �޽� /////
	// ���� �Ľ��ϰ� �ִ� �޽� ���ʹ� �ʱ�ȭ
	m_FBXMeshVec.clear();

	// ���ø��� ���� �ʵ� �ʱ�ȭ
	m_CheckSplitedVertexMap.clear();

	// �ִϸ��̼� /////
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

		// ��尡 �޽�, ���� ��츸 �޽��� ����, (Camera�� Light�� �ǳʶڴ�.)
		if (_type == FbxNodeAttribute::eMesh
			|| _type == FbxNodeAttribute::eSkeleton)
		{
			std::unique_ptr<ParserData::Mesh> _mesh = std::make_unique<ParserData::Mesh>();

			// �ϴ�, ��� = �޽���� ����
			// ���� ����� �̸�
			_mesh->m_NodeName = node->GetName();

			// �θ� �ִٸ�
			if (node->GetParent() != nullptr)
			{
				_mesh->m_NodeParentName = node->GetParent()->GetName();
			}

			/// Negative Scale
			//CheckNegativeScale(_mesh);

			// Bone�� �ƴ� �޽��� ���
			if (_type == FbxNodeAttribute::eMesh)
			{
				FbxMesh* _fbxMesh = node->GetMesh();

				/// �ִϸ��̼��� �ִ��� �˻�
				int _deformerCount = _fbxMesh->GetDeformerCount();

				// Deformer(Bone)�� ������ 1�� �̻��̸� �� Fbx�� �ִϸ��̼� �����Ͱ� �ְ�, ���� �޽��� SkinnedMesh�̴�.
				if (_deformerCount > 0)
				{
					// �ִϸ��̼� �����Ͱ� �����Ƿ� ����� �������ش�.
					if (m_NowMotion == nullptr)
					{
						m_NowMotion = std::make_unique<Motion>();
					}

					// �� �������� ��� �ð��� �־���
					m_NowMotion->m_TicksPerFrame = static_cast<float>(m_TicksPerFrame);

					// �ִϸ��̼��� �ִٸ� Skinned Mesh�� üũ���ְ�
					// ���� ��� ���� �Ŀ� Skinned Mesh �Ľ��� �����Ѵ�.
					_mesh->m_IsSkinnedMesh = true;
					
					m_pSkinnedMeshFbxNode = node;
					m_pSkinnedMesh = _mesh.get();
				}
				else
				{
					/// Pos
					CreateVertex_And_LoadVertexPos(node, _mesh.get());	// �� �Լ��� ������ FBXParserData::Mesh�� Vertex�� ��� �����Ǿ�����

					/// Normal, UV, Tangent...
					LoadVertexOthers(node, _mesh.get());				// ���⼭ Face�� ��ȸ�ϸ鼭 Vertex�� ������ �������� �ְ�, Index�� �־���
				}
			}
			// Bone�� ���
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

		/// Pos�� y,z ��������
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

		/// Pos�� y,z ��������
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

				// Negative Scale�� ��� Normal�� �������ش�.
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
		// map�� vertex�� �̹� �����Ѵ�.
		mesh->m_OptIndexVec.push_back(findVertex->second);
	}
	else
	{
		// map�� vertex�� �������� �ʴ´�.
		// �������� ������ ���ο� ������ �����, index�� ����
		size_t _vertexIndex = m_CheckSplitedVertexMap.size();
		mesh->m_OptIndexVec.push_back(_vertexIndex);
		
		mesh->m_OptVertexVec.push_back(vertex);
		m_CheckSplitedVertexMap[vertex] = _vertexIndex;
	}
	*/
	/// ���ø� ���Ϸ���
	mesh->m_OptIndexVec.push_back(mesh->m_OptVertexVec.size());
	mesh->m_OptVertexVec.push_back(vertex);
}

void FBXParser::CheckNegativeScale(ParserData::Mesh* mesh)
{
	// [ Negative Scale Ȯ�� ��� ]
	// row0, row1, row2�� ���� x,y,z���� ��Ÿ���Ƿ� �� �� �� ���� '����'�� ���� �ٸ� �� ��� ������ �ݴ����� Ȥ�� �������� Ȯ���ϸ�ȴ�.
	// -> ���� �� ���� �ٸ� �� ���� �����Ѵ�. (������ ��� ���� �ݴ�, ����� ��� ���� ���� ����)
	EMath::Vector3 _row0(mesh->m_NodeTM._11, mesh->m_NodeTM._12, mesh->m_NodeTM._13);
	EMath::Vector3 _row1(mesh->m_NodeTM._21, mesh->m_NodeTM._22, mesh->m_NodeTM._23);
	EMath::Vector3 _row2(mesh->m_NodeTM._31, mesh->m_NodeTM._32, mesh->m_NodeTM._33);

	EMath::Vector3 _crossResult = _row0.Cross(_row1);
	float checkNegativeScaleResult = _crossResult.Dot(_row2);

	// ������ ���� �ݴ��̴�.
	if (checkNegativeScaleResult < 0)
	{
		mesh->m_IsNegativeScale = true;

		// NodeTM
		EMath::Vector3 _scale;
		EMath::Quaternion _rot;
		EMath::Vector3 _trans;
		mesh->m_NodeTM.Decompose(_scale, _rot, _trans);

		// �������� ��������
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
		
		// �������� ��������
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
	// FBX������ mm������ ��������. ��, 1m�� �������� 1000mm???
	// �ƹ�ư ����Ƽ������ �������� ������ 100�� ����Ƽ���� ������ 1�� ����. (Down Scale�� ����)

	/// World
	EMath::Vector3 _scale;
	EMath::Quaternion _rot;
	EMath::Vector3 _trans;
	mesh->m_NodeTM.Decompose(_scale, _rot, _trans);

	// ������ ���
	_scale *= 0.01f;

	EMath::Matrix _scaleTM = EMath::Matrix::CreateScale(_scale);
	EMath::Matrix _rotTM = EMath::Matrix::CreateFromQuaternion(_rot);
	EMath::Matrix _transTM = EMath::Matrix::CreateTranslation(_trans);

	mesh->m_NodeTM = _scaleTM * _rotTM * _transTM;

	/// Local
	mesh->m_LocalTM.Decompose(_scale, _rot, _trans);

	// ������ ���
	_scale *= 0.01f;

	_scaleTM = EMath::Matrix::CreateScale(_scale);
	_rotTM = EMath::Matrix::CreateFromQuaternion(_rot);
	_transTM = EMath::Matrix::CreateTranslation(_trans);

	mesh->m_LocalTM = _scaleTM * _rotTM * _transTM;
}

void FBXParser::LoadSkinnedMeshData(FbxScene* scene)
{
	// Skinned Mesh�� ���� �Ľ� ����

	/// Pos
	CreateVertex_And_LoadVertexPos_Animation(m_pSkinnedMeshFbxNode, m_pSkinnedMesh);		// �� �Լ��� ������ FBXParserData::Mesh�� Vertex�� ��� �����Ǿ�����

	/// Weights
	LoadBoneWeights(m_pSkinnedMeshFbxNode, m_pSkinnedMesh);

	/// Normal, UV, Tangent...
	LoadVertexOthers(m_pSkinnedMeshFbxNode, m_pSkinnedMesh);					// ���⼭ Face�� ��ȸ�ϸ鼭 Vertex�� ������ �������� �ְ�, Index�� �־���
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

					// Bone Matrix ����
					EMath::Matrix clusterMatrix = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(_matClusterTransformMatrix);
					EMath::Matrix clusterlinkMatrix = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(_matClusterLinkTransformMatrix);
					EMath::Matrix geometryMatrix = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(_geometryTransform);

					EMath::Matrix offsetMatrix = clusterlinkMatrix.Invert() * clusterMatrix *  geometryMatrix;

					//m_pNowMotion->m_BoneOffsetTransformVec[boneIndex] = m_BoneVec[boneIndex]->m_NodeTM;
					m_NowMotion->m_BoneOffsetName.push_back(_linkFbxNodeName);
				}

				// ����ġ 1��?
				// ...
				// ...
			}
		}
	}
}

void FBXParser::LoadAnimationData(FbxScene* scene, FbxNode* node)
{
	/// �����κ��� �ִϸ��̼� �����͸� �ε��Ѵ�.
	// �ִϸ��̼� ������ ����
	std::unique_ptr<MeshAnimationData> _meshAnimationData = std::make_unique<MeshAnimationData>();

	// ���߿� ��� �������� �޽��� �����Ű�� ���� �̸��� ����
	_meshAnimationData->m_NodeName = node->GetName();

	for (FbxLongLong i = 0; i < m_TotalFrame; i++)
	{
		FbxTime _takeTime;
		_takeTime.SetFrame(m_StartFrame + i, m_FbxTimeMode);

		// ���� �������� ���� Ʈ������
		FbxAMatrix _animFbxTM = node->EvaluateLocalTransform(_takeTime);

		EMath::Matrix _animTM = MathConverter_Fbx::FbxMatrix_To_EMathMatrix(_animFbxTM, true);

		EMath::Vector3 _scale;
		EMath::Quaternion _rot;
		EMath::Vector3 _trans;

		_animTM.Decompose(_scale, _rot, _trans);

		// Pos Ű������, Pos Sample �ֱ�
		//_meshAnimationData->m_PosKeyFrameVec.push_back(i);
		//_meshAnimationData->m_PosSampleVec.push_back(_trans);

		// Rotation Ű������, Rot Sample �ֱ�
		//_meshAnimationData->m_RotKeyFrameVec.push_back(i);
		//_meshAnimationData->m_RotSampleVec.push_back(_rot);

		// Local Transform�� �ٷ� �ֱ�
		_meshAnimationData->m_AnimationTMVec.push_back(_animTM);
	}

	// �� ���� �ִϸ��̼� �����͸� ��ǿ� �ִ´�.
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
