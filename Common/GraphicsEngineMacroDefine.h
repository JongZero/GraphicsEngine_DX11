#pragma once
#include <windows.h>
#include <string>

#include "ObjectMeshInfo.h"
#include "Shared_RenderingData.h"

#ifdef GRAPHICSENGINE_DX11_EXPORTS
#define G_API __declspec(dllexport)
#else
#define G_API __declspec(dllimport)
#endif

// .fbx, .erj 중 어떤 파일인지
enum class ParserType
{
	FBX,
	ERJ,
};

extern "C"
{
	/// 그래픽스 엔진 생성 & 해제
	G_API void CreateGraphicsEngine(void** pReturnPointer, HWND hWnd, int clientWidth, int clientHeight);
	G_API void ReleaseGraphicsEngine();

	/// 리소스
	// 리소스의 개수를 체크하고, 파일의 경로를 저장해놓는다.
	G_API void LoadAllResourcesCount(std::wstring folderPath, ParserType type);
	// 리소스를 실질적으로 로드한다.
	G_API void LoadAllResources();

	/// 창 크기 설정 & 재초기화
	G_API void SetClientSize(int clientWidth, int clientHeight);
	G_API void OnResize();

	/// 메쉬
	// 오브젝트의 ID와 메쉬에 대한 견적서를 받아 메쉬를 생성하고 리소스 매니저에 보관
	G_API void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo);
	// 리소스 매니저에 보관되어있는 메쉬 데이터를 삭제
	G_API void DeleteMeshes(std::wstring objName);

	/// 애니메이션
	G_API void InitializeAnimation(std::wstring objName, bool& isCompleted);
	G_API void UpdateAnimation(std::wstring objName, std::wstring nowMotionName, float dTime, float speed);

	/// 렌더링
	// 렌더링을 시작하라고 게임 엔진에서 호출하는 함수 (데이터만 넘겨주면 그래픽스 엔진에서 알아서 렌더링한다.)
	G_API void StartRender(Shared_RenderingData* dataForRender);

	/// 피킹
	G_API void PickCheck(int x, int y);

	/// 파티클 시스템
	G_API void InitializeParticleSystem(Shared_ParticleSystemData* pData);
	G_API void ReleaseParticleSystem(std::wstring name);
}

/// 그래픽스 엔진 생성 & 해제
typedef void (*CreateGraphicsEngineDLL)(void** pReturnPointer, HWND hWnd, int clientWidth, int clientHeight);
typedef void (*ReleaseGraphicsEngineDLL)();

/// 리소스
typedef void (*LoadAllResourcesCountDLL)(std::wstring folderPath, ParserType type);
typedef void (*LoadAllResourcesDLL)();

/// 창 크기 설정 & 재초기화
typedef void (*SetClientSizeDLL)(int clientWidth, int clientHeight);
typedef void (*OnResizeDLL)();

/// 메쉬
typedef void (*CreateMeshesDLL)(std::wstring objName, ObjectMeshInfo& objectMeshInfo);
typedef void (*DeleteMeshesDLL)(std::wstring objName);

/// 애니메이션
typedef void (*InitializeAnimationDLL)(std::wstring objName, bool& isCompleted);
typedef void (*UpdateAnimationDLL)(std::wstring objName, std::wstring nowMotionName, float dTime, float speed);

/// 렌더링
typedef void (*StartRenderDLL)(Shared_RenderingData* dataForRender);

/// 피킹
typedef void (*PickCheckDLL)(int x, int y);

/// 파티클 시스템
typedef void (*InitializeParticleSystemDLL)(Shared_ParticleSystemData* pData);
typedef void (*ReleaseParticleSystemDLL)(std::wstring name);

namespace FuncVariable
{
	/// 그래픽스 엔진 생성 & 해제
	static CreateGraphicsEngineDLL CreateGraphicsEngine;
	static ReleaseGraphicsEngineDLL ReleaseGraphicsEngine;

	/// 리소스
	static LoadAllResourcesCountDLL LoadAllResourcesCount;
	static LoadAllResourcesDLL LoadAllResources;

	/// 창 크기 설정 & 재초기화
	static SetClientSizeDLL SetClientSize;
	static OnResizeDLL OnResize;

	/// 메쉬
	static CreateMeshesDLL CreateMeshes;
	static DeleteMeshesDLL DeleteMeshes;

	/// 애니메이션
	static InitializeAnimationDLL InitializeAnimation;
	static UpdateAnimationDLL UpdateAnimation;

	/// 렌더링
	static StartRenderDLL StartRender;

	/// 피킹
	static PickCheckDLL PickCheck;

	/// 파티클 시스템
	static InitializeParticleSystemDLL InitializeParticleSystem;
	static ReleaseParticleSystemDLL ReleaseParticleSystem;
}

/// Dll을 한 번 로드해서 계속 메모리 상에 들고있고(static) 그래픽스 엔진을 셧다운할 때 FreeLibrary로 해제한다.
static HINSTANCE g_hDll = LoadLibrary(L"../Lib/GraphicsEngine_DX11_x64_debug.dll");

// 이 Define Macro를 활용해서 그래픽스 엔진 함수를 사용할 수도 있고(그래픽스 엔진 포인터가 필요없다.)
// 그래픽스 엔진 인터페이스의 포인터로 그래픽스 엔진에 바로 접근할 수도 있다.
#define GRAPHICS_FUNC(Func_Name, ...)\
if(g_hDll)\
{\
	FuncVariable::Func_Name = (Func_Name##DLL)::GetProcAddress(g_hDll, #Func_Name);\
	FuncVariable::Func_Name(##__VA_ARGS__);\
}
