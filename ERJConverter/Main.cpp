#include "ERJConverter.h"
#include "FBXParser.h"
#include "MeshAnimationData.h"
#include <memory>
#include <filesystem>

// Utility - For EMath
#ifdef _WIN64
	#ifdef _DEBUG
		#pragma comment(lib, "Utility_x64_debug.lib")
	#else
		#pragma comment(lib, "Utility_x64_release.lib")
	#endif
#else
	#ifdef _DEBUG
		#pragma comment(lib, "Utility_x86_debug.lib")
	#else
		#pragma comment(lib, "Utility_x86_release.lib")
	#endif
#endif

// FBX Parser
std::unique_ptr<FBXParser> g_FBXParser;

// FBX ������ ����
int g_AllResourcesCount = 0;

// C++ 17, FBX�� �Ľ��ϱ� ���� ��ο� �̸��� �����س���, �̸� ������ �ľ��س��´�. (����� ��Ȳ�� ���ؼ�)
void LoadFilePathAndName_Recursive(std::wstring folderPath, std::unordered_map<std::wstring, std::wstring>& map)
{
	const std::filesystem::path _folderPath{ folderPath.c_str() };
	std::filesystem::create_directories(_folderPath);

	for (const auto& file : std::filesystem::recursive_directory_iterator{ _folderPath })
	{
		std::wstring filePath = file.path();
		std::wstring fileName = filePath.substr(filePath.rfind(L"/") + 1, filePath.rfind(L"."));

		// Ȯ���� ����
		fileName = fileName.substr(0, fileName.size() - 4);

		// file path & file name ����
		map[filePath] = fileName;
	}

	g_AllResourcesCount += map.size();
}

/// <summary>
/// FBX Parser�� ���� FBX�� �а� �Ľ��� �� ���� ���� ����ü�� �����͸� ������ �� ERJ ���Ϸ� ��ȯ�Ѵ�.
/// 1���� FBX ���� -> 1���� ERJ ���Ϸ� ��ȯ�Ѵ�.
/// �̸� ���� Trianglualte, Tangent Space ���ϱ� ���� ���ϸ鼭 �ӵ� ���� ���ȼ��� Ȯ���Ѵ�.
/// 2021. 02. 21 ������
/// </summary>
/// <returns></returns>
int main()
{
	std::cout << "==============================\n";
	std::cout << "ERJ Converter v0.3\n" << "2022. 02. 21 - ������\n";
	std::cout << "==============================\n\n";

	g_FBXParser = std::make_unique<FBXParser>();

	// ���� ���, �̸��� �� ��
	std::unordered_map<std::wstring, std::wstring>& _map = g_FBXParser->GetFilePathAndFileNameUMap_FBX();

	// ���� ���, �̸��� �ε�
	LoadFilePathAndName_Recursive(L"../Data/FBX/", _map);

	// �ε尡 ������ ������ �� �� ����
	std::cout << "FBX ������ �� ���� : " << g_AllResourcesCount << "\n\n";

	// ������ �о���� ���� ��ο� �̸��� �������� ���� FBX �ε�
	g_FBXParser->LoadAllFBXFiles(true);
	
	// FBX�� �Ľ��� �����͸� ERJ�� ����
	int _count = 0;
	ERJConverter _erjConverter;
	std::wstring _folderPath = L"../Data/ERJ/";

	for (const auto& it : _map)
	{
		std::string _filePath(_folderPath.begin(), _folderPath.end());
		std::string _fileName(it.second.begin(), it.second.end());

		_filePath += _fileName;
		_filePath += ".erj";

		std::cout << _filePath << " ��ȯ ��...\n";

		std::ofstream _os(_filePath, std::ios::binary);

		if (!_os.is_open())
		{
			std::cout << "[error]" << _filePath << "���� ���� ����\n";
		}
		
		// �޽�
		std::vector<ParserData::Mesh*> _meshVec = g_FBXParser->GetMeshVec(it.second);

		unsigned int _meshVecSize = _meshVec.size();
		if (_meshVecSize > 0)
		{
			// ������ ����� �� �� (unsigned int)�� ĳ�������� ������
			// 64��Ʈ ȯ�濡���� (unsigned int_64)�� �����ع����� ������ 8����Ʈ��ŭ �����ع�����.
			// 4����Ʈ�� ����ϴ�.
			_erjConverter.WriteToERJ(_os, _meshVecSize);

			for (int i = 0; i < _meshVecSize > 0; i++)
			{
				// Vertex
				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_OptVertexVec);

				int a = sizeof(_meshVec[i]->m_OptVertexVec);
				int b = 0;

				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_OptIndexVec);

				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_NodeName);
				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_NodeParentName);

				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_NodeTM);
				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_NodeInverseTM);
				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_LocalTM);

				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_FaceCount);

				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_IsBone);
				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_BoneIndex);

				_erjConverter.WriteToERJ(_os, _meshVec[i]->m_IsSkinnedMesh);
			}
		}

		// �ִϸ��̼�
		std::unordered_map<std::wstring, Motion*> _motionUMap = g_FBXParser->GetObjectMotionData(it.second);

		unsigned int _motionUMapSize = _motionUMap.size();
		_erjConverter.WriteToERJ(_os, _motionUMapSize);
		if (_motionUMapSize > 0)
		{
			for (const auto& motion : _motionUMap)
			{
				// First Node Name (������Ʈ ���п�)
				_erjConverter.WriteToERJ(_os, motion.second->m_ObjectNodeName);

				// Motion Name
				_erjConverter.WriteToERJ(_os, motion.second->m_MotionName);

				// Animation Data Vector
				unsigned int _animDataVecSize = motion.second->m_AnimationDataVec.size();
				_erjConverter.WriteToERJ(_os, _animDataVecSize);
				for (const auto& animData : motion.second->m_AnimationDataVec)
				{
					// Node Name
					_erjConverter.WriteToERJ(_os, animData->m_NodeName);

					// Animation TM
					unsigned int _animTMVecSize = animData->m_AnimationTMVec.size();
					_erjConverter.WriteToERJ(_os, _animTMVecSize);
					for (const auto& animTM : animData->m_AnimationTMVec)
					{
						_erjConverter.WriteToERJ(_os, animTM);
					}
				}

				// TicksPerFrame
				_erjConverter.WriteToERJ(_os, motion.second->m_TicksPerFrame);
			}
		}

		_count++;
		float _per = (float)_count / g_AllResourcesCount * 100.0f;

		std::cout << _filePath << " ��ȯ �Ϸ�\n" << "����� : " << _per << "%\n\n";
		_os.close();
	}

	system("pause");
	return 0;
}
