#include "ERJParser.h"
#include <iostream>
#include <fstream>

ERJParser::ERJParser()
{
	m_ERJConverter = std::make_unique<ERJConverter>();
}

ERJParser::~ERJParser()
{

}

std::vector<ParserData::Mesh*> ERJParser::GetMeshVec(std::wstring fileName)
{
	if (m_MeshVecUMap.count(fileName) > 0)
	{
		return m_MeshVecUMap[fileName];
	}
	else
	{
		std::vector<ParserData::Mesh*> _fail;
		return _fail;
	}
}

std::unordered_map<std::wstring, Motion*> ERJParser::GetObjectMotionData(std::wstring name)
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

void ERJParser::LoadAllERJFiles(bool isPrint /*= false*/)
{
	// ERJ ���� ��θ� ��ȸ�ϸ鼭 �ϳ��� �ε�
	for (const auto& it : m_FilePathAndFileNameMap_ERJ)
	{
		LoadFile(it);
	}
}

void ERJParser::LoadFile(std::pair<std::wstring, std::wstring> pathAndName)
{
	std::string _path(pathAndName.first.begin(), pathAndName.first.end());

	std::ifstream _is(_path.c_str(), std::ios::binary);

	if (!_is.is_open())
	{
		std::cout << "[error]" << _path.c_str() << " ���� ���� ����\n";
	}

	unsigned int _meshVecSize = 0;
	m_ERJConverter->ReadFromERJ(_is, _meshVecSize);

	std::vector<ParserData::Mesh*> _meshVec;

	for (int i = 0; i < _meshVecSize > 0; i++)
	{
		std::unique_ptr<ParserData::Mesh> _mesh = std::make_unique<ParserData::Mesh>();

		// Vertex
		unsigned int _vertexVecSize = 0;
		m_ERJConverter->ReadFromERJ(_is, _vertexVecSize);
		_mesh->m_OptVertexVec.resize(_vertexVecSize);
		
		for (int j = 0; j < _vertexVecSize; j++)
		{
			m_ERJConverter->ReadFromERJ(_is, _mesh->m_OptVertexVec[j]);
		}

		m_ERJConverter->ReadFromERJ(_is, _mesh->m_OptIndexVec);

		m_ERJConverter->ReadFromERJ(_is, _mesh->m_NodeName);
		m_ERJConverter->ReadFromERJ(_is, _mesh->m_NodeParentName);

		m_ERJConverter->ReadFromERJ(_is, _mesh->m_NodeTM);
		m_ERJConverter->ReadFromERJ(_is, _mesh->m_NodeInverseTM);
		m_ERJConverter->ReadFromERJ(_is, _mesh->m_LocalTM);

		m_ERJConverter->ReadFromERJ(_is, _mesh->m_FaceCount);

		m_ERJConverter->ReadFromERJ(_is, _mesh->m_IsBone);
		m_ERJConverter->ReadFromERJ(_is, _mesh->m_BoneIndex);

		m_ERJConverter->ReadFromERJ(_is, _mesh->m_IsSkinnedMesh);
		_meshVec.push_back(_mesh.get());
		m_UniqueMeshVec.push_back(std::move(_mesh));
	}

	// Mesh�� �ִٸ�
	if (_meshVecSize > 0)
	{
		m_MeshVecUMap[pathAndName.second] = _meshVec;
	}

	// �ִϸ��̼�
	unsigned int _motionUMapSize = 0;
	m_ERJConverter->ReadFromERJ(_is, _motionUMapSize);
	for (int i = 0; i < _motionUMapSize; i++)
	{
		std::unique_ptr<Motion> _motion = std::make_unique<Motion>();

		// First Node Name (������Ʈ ���п�)
		m_ERJConverter->ReadFromERJ(_is, _motion->m_ObjectNodeName);

		// Motion Name
		m_ERJConverter->ReadFromERJ(_is, _motion->m_MotionName);

		// Animation Data Vector
		unsigned int _animDataVecSize = 0;
		m_ERJConverter->ReadFromERJ(_is, _animDataVecSize);
		for (int j = 0; j < _animDataVecSize; j++)
		{
			std::unique_ptr<MeshAnimationData> _animData = std::make_unique<MeshAnimationData>();

			// Node Name
			m_ERJConverter->ReadFromERJ(_is, _animData->m_NodeName);

			// Animation TM
			unsigned int _animTMVecSize = 0;
			m_ERJConverter->ReadFromERJ(_is, _animTMVecSize);
			for (int k = 0; k < _animTMVecSize; k++)
			{
				EMath::Matrix _animTM;
				m_ERJConverter->ReadFromERJ(_is, _animTM);

				_animData->m_AnimationTMVec.push_back(_animTM);
			}
			
			_motion->m_AnimationDataVec.push_back(_animData.get());
			m_UniqueAnimDataVec.push_back(std::move(_animData));
		}

		// TicksPerFrame
		m_ERJConverter->ReadFromERJ(_is, _motion->m_TicksPerFrame);

		std::wstring _motionName(_motion->m_MotionName.begin(), _motion->m_MotionName.end());

		/// ���� ������Ʈ���� üũ�ؾ���
		std::unordered_map<std::wstring, Motion*> _motionMap = m_ObjectMotionUMap[pathAndName.second];

		// ���� �̸��� ������Ʈ�� �ʿ� ���ٸ�
		if (_motionMap.size() <= 0)
		{
			_motionMap[_motionName] = _motion.get();
			m_ObjectMotionUMap[pathAndName.second] = _motionMap;
		}
		// �̹� �ִٸ� ���� ���Ϳ� �ִ´�. (���� ������Ʈ�̹Ƿ�)
		else
		{
			m_ObjectMotionUMap[pathAndName.second][_motionName] = _motion.get();
		}

		m_UniqueMotionVec.push_back(std::move(_motion));
	}
}
