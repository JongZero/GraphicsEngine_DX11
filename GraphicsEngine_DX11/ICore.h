#pragma once
#include <windows.h>

/// <summary>
/// ������ ������ �ھ�
/// DX, OpenGL�� ���� �׷��� ���̺귯���� �������� �κ��� ��� �� ������ ó���Ѵ�.
/// DX, OpenGL ��� ȣȯ�ǵ��� ������ �;����� �������
/// 2021. 07. 19 ������
/// DX11, DX12�� ���� �����ǵ��� �Ϸ�����
/// 2021. 10. 26 ������
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