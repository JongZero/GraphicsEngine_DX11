#pragma once
#include <vector>

class DX11Core;
class RenderTarget;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

/// <summary>
/// Skybox에서 쓰는 Cubemap
/// 2022. 03. 31 정종영
/// </summary>
class Cubemap
{
public:
	Cubemap(DX11Core* dx11Core, std::vector<RenderTarget*> faces, int width, int height, int mipMaps);
	~Cubemap();

private:
	ID3D11Texture2D* m_pTexture;
	ID3D11ShaderResourceView* m_pSRV;
	int m_MipMaps;

public:
	ID3D11Texture2D* GetTexture();
	ID3D11ShaderResourceView* GetSRV();
	
public:
	void Initialize(DX11Core* dx11Core, std::vector<RenderTarget*> faces, int width, int height, int mipMaps);
	void Copy(DX11Core* dx11Core, std::vector<RenderTarget*> faces, int width, int height, int mipSlice);
};

