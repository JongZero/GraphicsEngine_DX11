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

// .fbx, .erj �� � ��������
enum class ParserType
{
	FBX,
	ERJ,
};

extern "C"
{
	/// �׷��Ƚ� ���� ���� & ����
	G_API void CreateGraphicsEngine(void** pReturnPointer, HWND hWnd, int clientWidth, int clientHeight);
	G_API void ReleaseGraphicsEngine();

	/// ���ҽ�
	// ���ҽ��� ������ üũ�ϰ�, ������ ��θ� �����س��´�.
	G_API void LoadAllResourcesCount(std::wstring folderPath, ParserType type);
	// ���ҽ��� ���������� �ε��Ѵ�.
	G_API void LoadAllResources();

	/// â ũ�� ���� & ���ʱ�ȭ
	G_API void SetClientSize(int clientWidth, int clientHeight);
	G_API void OnResize();

	/// �޽�
	// ������Ʈ�� ID�� �޽��� ���� �������� �޾� �޽��� �����ϰ� ���ҽ� �Ŵ����� ����
	G_API void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo);
	// ���ҽ� �Ŵ����� �����Ǿ��ִ� �޽� �����͸� ����
	G_API void DeleteMeshes(std::wstring objName);

	/// �ִϸ��̼�
	G_API void InitializeAnimation(std::wstring objName, bool& isCompleted);
	G_API void UpdateAnimation(std::wstring objName, std::wstring nowMotionName, float dTime, float speed);

	/// ������
	// �������� �����϶�� ���� �������� ȣ���ϴ� �Լ� (�����͸� �Ѱ��ָ� �׷��Ƚ� �������� �˾Ƽ� �������Ѵ�.)
	G_API void StartRender(Shared_RenderingData* dataForRender);

	/// ��ŷ
	G_API void PickCheck(int x, int y);

	/// ��ƼŬ �ý���
	G_API void InitializeParticleSystem(Shared_ParticleSystemData* pData);
	G_API void ReleaseParticleSystem(std::wstring name);
}

/// �׷��Ƚ� ���� ���� & ����
typedef void (*CreateGraphicsEngineDLL)(void** pReturnPointer, HWND hWnd, int clientWidth, int clientHeight);
typedef void (*ReleaseGraphicsEngineDLL)();

/// ���ҽ�
typedef void (*LoadAllResourcesCountDLL)(std::wstring folderPath, ParserType type);
typedef void (*LoadAllResourcesDLL)();

/// â ũ�� ���� & ���ʱ�ȭ
typedef void (*SetClientSizeDLL)(int clientWidth, int clientHeight);
typedef void (*OnResizeDLL)();

/// �޽�
typedef void (*CreateMeshesDLL)(std::wstring objName, ObjectMeshInfo& objectMeshInfo);
typedef void (*DeleteMeshesDLL)(std::wstring objName);

/// �ִϸ��̼�
typedef void (*InitializeAnimationDLL)(std::wstring objName, bool& isCompleted);
typedef void (*UpdateAnimationDLL)(std::wstring objName, std::wstring nowMotionName, float dTime, float speed);

/// ������
typedef void (*StartRenderDLL)(Shared_RenderingData* dataForRender);

/// ��ŷ
typedef void (*PickCheckDLL)(int x, int y);

/// ��ƼŬ �ý���
typedef void (*InitializeParticleSystemDLL)(Shared_ParticleSystemData* pData);
typedef void (*ReleaseParticleSystemDLL)(std::wstring name);

namespace FuncVariable
{
	/// �׷��Ƚ� ���� ���� & ����
	static CreateGraphicsEngineDLL CreateGraphicsEngine;
	static ReleaseGraphicsEngineDLL ReleaseGraphicsEngine;

	/// ���ҽ�
	static LoadAllResourcesCountDLL LoadAllResourcesCount;
	static LoadAllResourcesDLL LoadAllResources;

	/// â ũ�� ���� & ���ʱ�ȭ
	static SetClientSizeDLL SetClientSize;
	static OnResizeDLL OnResize;

	/// �޽�
	static CreateMeshesDLL CreateMeshes;
	static DeleteMeshesDLL DeleteMeshes;

	/// �ִϸ��̼�
	static InitializeAnimationDLL InitializeAnimation;
	static UpdateAnimationDLL UpdateAnimation;

	/// ������
	static StartRenderDLL StartRender;

	/// ��ŷ
	static PickCheckDLL PickCheck;

	/// ��ƼŬ �ý���
	static InitializeParticleSystemDLL InitializeParticleSystem;
	static ReleaseParticleSystemDLL ReleaseParticleSystem;
}

/// Dll�� �� �� �ε��ؼ� ��� �޸� �� ����ְ�(static) �׷��Ƚ� ������ �˴ٿ��� �� FreeLibrary�� �����Ѵ�.
static HINSTANCE g_hDll = LoadLibrary(L"../Lib/GraphicsEngine_DX11_x64_debug.dll");

// �� Define Macro�� Ȱ���ؼ� �׷��Ƚ� ���� �Լ��� ����� ���� �ְ�(�׷��Ƚ� ���� �����Ͱ� �ʿ����.)
// �׷��Ƚ� ���� �������̽��� �����ͷ� �׷��Ƚ� ������ �ٷ� ������ ���� �ִ�.
#define GRAPHICS_FUNC(Func_Name, ...)\
if(g_hDll)\
{\
	FuncVariable::Func_Name = (Func_Name##DLL)::GetProcAddress(g_hDll, #Func_Name);\
	FuncVariable::Func_Name(##__VA_ARGS__);\
}
