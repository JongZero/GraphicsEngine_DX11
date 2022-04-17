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

// FBX 파일의 개수
int g_AllResourcesCount = 0;

// C++ 17, FBX를 파싱하기 전에 경로와 이름을 저장해놓고, 미리 개수를 파악해놓는다. (진행률 현황을 위해서)
void LoadFilePathAndName_Recursive(std::wstring folderPath, std::unordered_map<std::wstring, std::wstring>& map)
{
	const std::filesystem::path _folderPath{ folderPath.c_str() };
	std::filesystem::create_directories(_folderPath);

	for (const auto& file : std::filesystem::recursive_directory_iterator{ _folderPath })
	{
		std::wstring filePath = file.path();
		std::wstring fileName = filePath.substr(filePath.rfind(L"/") + 1, filePath.rfind(L"."));

		// 확장자 제거
		fileName = fileName.substr(0, fileName.size() - 4);

		// file path & file name 저장
		map[filePath] = fileName;
	}

	g_AllResourcesCount += map.size();
}

/// <summary>
/// FBX Parser를 통해 FBX를 읽고 파싱한 후 내가 만든 구조체로 데이터를 정렬한 뒤 ERJ 파일로 변환한다.
/// 1개의 FBX 파일 -> 1개의 ERJ 파일로 변환한다.
/// 이를 통해 Trianglualte, Tangent Space 구하기 등을 안하면서 속도 향상과 보안성을 확보한다.
/// 2021. 02. 21 정종영
/// </summary>
/// <returns></returns>
int main()
{
	std::cout << "==============================\n";
	std::cout << "ERJ Converter v0.3\n" << "2022. 02. 21 - 정종영\n";
	std::cout << "==============================\n\n";

	g_FBXParser = std::make_unique<FBXParser>();

	// 파일 경로, 이름이 들어갈 맵
	std::unordered_map<std::wstring, std::wstring>& _map = g_FBXParser->GetFilePathAndFileNameUMap_FBX();

	// 파일 경로, 이름을 로드
	LoadFilePathAndName_Recursive(L"../Data/FBX/", _map);

	// 로드가 끝나면 개수를 알 수 있음
	std::cout << "FBX 파일의 총 개수 : " << g_AllResourcesCount << "\n\n";

	// 위에서 읽어들인 파일 경로와 이름을 바탕으로 실제 FBX 로드
	g_FBXParser->LoadAllFBXFiles(true);
	
	// FBX를 파싱한 데이터를 ERJ로 저장
	int _count = 0;
	ERJConverter _erjConverter;
	std::wstring _folderPath = L"../Data/ERJ/";

	for (const auto& it : _map)
	{
		std::string _filePath(_folderPath.begin(), _folderPath.end());
		std::string _fileName(it.second.begin(), it.second.end());

		_filePath += _fileName;
		_filePath += ".erj";

		std::cout << _filePath << " 변환 중...\n";

		std::ofstream _os(_filePath, std::ios::binary);

		if (!_os.is_open())
		{
			std::cout << "[error]" << _filePath << "파일 오픈 실패\n";
		}
		
		// 메쉬
		std::vector<ParserData::Mesh*> _meshVec = g_FBXParser->GetMeshVec(it.second);

		unsigned int _meshVecSize = _meshVec.size();
		if (_meshVecSize > 0)
		{
			// 벡터의 사이즈를 줄 때 (unsigned int)로 캐스팅하지 않으면
			// 64비트 환경에서는 (unsigned int_64)를 저장해버리기 때문에 8바이트만큼 저장해버린다.
			// 4바이트로 충분하다.
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

		// 애니메이션
		std::unordered_map<std::wstring, Motion*> _motionUMap = g_FBXParser->GetObjectMotionData(it.second);

		unsigned int _motionUMapSize = _motionUMap.size();
		_erjConverter.WriteToERJ(_os, _motionUMapSize);
		if (_motionUMapSize > 0)
		{
			for (const auto& motion : _motionUMap)
			{
				// First Node Name (오브젝트 구분용)
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

		std::cout << _filePath << " 변환 완료\n" << "진행률 : " << _per << "%\n\n";
		_os.close();
	}

	system("pause");
	return 0;
}
