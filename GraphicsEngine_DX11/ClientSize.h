#pragma once

/// <summary>
/// 클라이언트의 크기를 전역적으로 쓰기위해 만든 클래스
/// Set은 엔진에서만, 이외의 클래스들은 Get만 가능하다.
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
	static float AspectRatio();		// 종횡비(r)
};
