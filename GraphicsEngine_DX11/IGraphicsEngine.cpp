#include "GraphicsEngine.h"

/// dll 사용자가 사용할 그래픽스 엔진 포인터
static GraphicsEngine* g_GraphicsEngine = nullptr;

G_API void CreateGraphicsEngine(void** pReturnPointer, HWND hWnd, int clientWidth, int clientHeight)
{
	g_GraphicsEngine = new GraphicsEngine(hWnd, clientWidth, clientHeight);

	*pReturnPointer = g_GraphicsEngine;
}

G_API void ReleaseGraphicsEngine()
{
	if (g_hDll != nullptr)
	{
		FreeLibrary(g_hDll);
	}

	if (g_GraphicsEngine != nullptr)
	{
		// 그래픽스 엔진 삭제 전 호출해야하는 함수들...
		// ex) 리소스 해제... 등등
		// ...

		delete g_GraphicsEngine;
		g_GraphicsEngine = nullptr;
	}
}

/// 매크로 사용을 위한 함수들 ///////////////////////////////////////////

G_API void CreateMeshes(std::wstring objName, ObjectMeshInfo& objectMeshInfo)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->CreateMeshes(objName, objectMeshInfo);
}

G_API void DeleteMeshes(std::wstring objName)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->DeleteMeshes(objName);
}

G_API void StartRender(Shared_RenderingData* dataForRender)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->StartRender(dataForRender);
}

G_API void LoadAllResourcesCount(std::wstring folderPath, ParserType type)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->LoadAllResourcesCount(folderPath, type);
}

G_API void LoadAllResources()
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->LoadAllResources();
}

G_API void SetClientSize(int clientWidth, int clientHeight)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->SetClientSize(clientWidth, clientHeight);
}

G_API void OnResize()
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->OnResize();
}

G_API void InitializeAnimation(std::wstring objName, bool& isCompleted)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->InitializeAnimation(objName, isCompleted);
}

G_API void UpdateAnimation(std::wstring objName, std::wstring nowMotionName, float dTime, float speed)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->UpdateAnimation(objName, nowMotionName, dTime, speed);
}

G_API void PickCheck(int x, int y)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->PickCheck(x, y);
}

G_API void InitializeParticleSystem(Shared_ParticleSystemData* pData)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->InitializeParticleSystem(pData);
}

G_API void ReleaseParticleSystem(std::wstring name)
{
	if (g_GraphicsEngine)
		g_GraphicsEngine->ReleaseParticleSystem(name);
}
