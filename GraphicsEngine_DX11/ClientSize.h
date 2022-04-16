#pragma once

/// <summary>
/// Ŭ���̾�Ʈ�� ũ�⸦ ���������� �������� ���� Ŭ����
/// Set�� ����������, �̿��� Ŭ�������� Get�� �����ϴ�.
/// 2021. 06. 30
/// </summary>
class ClientSize
{
	friend class GraphicsEngine;

private:
	static int Width;
	static int Height;

public:
	static int GetWidth() { return Width; }
	static int GetHeight() { return Height; }
	static float AspectRatio();		// ��Ⱦ��(r)
};
