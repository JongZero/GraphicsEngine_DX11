#pragma once
#include <windows.h>

/// <summary>
/// 렌더링 엔진의 코어
/// DX, OpenGL과 같은 그래픽 라이브러리에 종속적인 부분은 모두 이 곳에서 처리한다.
/// DX, OpenGL 모두 호환되도록 만들어보고 싶어져서 만들어짐
/// 2021. 07. 19 정종영
/// DX11, DX12가 같이 지원되도록 하려고함
/// 2021. 10. 26 정종영
/// </summary>
class ICore
{
public:
	virtual bool Initialize(HWND hWnd) abstract;
	virtual void OnResize() abstract;

public:
	virtual void ClearMainRenderTarget() abstract;
	virtual void ClearDepthBuffer() abstract;
	virtual void SetMainRenderTarget() abstract;
	virtual void SetMainViewport() abstract;
	virtual void SetDefaultStates() abstract;
	virtual void ClearShaderResources() abstract;
	virtual void Present() abstract;
	virtual void Debug() abstract;
};