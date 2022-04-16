#include "Cubemap.h"
#include "MacroDefine.h"
#include "RenderTarget.h"
#include "DX11Core.h"

Cubemap::Cubemap(DX11Core* dx11Core, std::vector<RenderTarget*> faces, int width, int height, int mipMaps)
	: m_MipMaps(mipMaps), m_pTexture(nullptr), m_pSRV(nullptr)
{
	Initialize(dx11Core, faces, width, height, mipMaps);
}

Cubemap::~Cubemap()
{
	if (m_pTexture)
	{
		m_pTexture->Release();
		m_pTexture = nullptr;
	}

	if (m_pSRV)
	{
		m_pSRV->Release();
		m_pSRV = nullptr;
	}
}

ID3D11Texture2D* Cubemap::GetTexture()
{
	return m_pTexture;
}

ID3D11ShaderResourceView* Cubemap::GetSRV()
{
	return m_pSRV;
}

void Cubemap::Initialize(DX11Core* dx11Core, std::vector<RenderTarget*> faces, int width, int height, int mipMaps)
{
	ID3D11Device* device = dx11Core->GetDevice();
	ID3D11DeviceContext* dc = dx11Core->GetDC();

	m_MipMaps = mipMaps;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = mipMaps;
	texDesc.ArraySize = 6;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	device->CreateTexture2D(&texDesc, nullptr, &m_pTexture);

	if (!faces.empty())
	{
		Copy(dx11Core, faces, width, height, 0);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;

	device->CreateShaderResourceView(m_pTexture, &srvDesc, &m_pSRV);
}

void Cubemap::Copy(DX11Core* dx11Core, std::vector<RenderTarget*> faces, int width, int height, int mipSlice)
{
	ID3D11DeviceContext* dc = dx11Core->GetDC();

	D3D11_BOX sourceRegion;
	for (int i = 0; i < 6; ++i)
	{
		RenderTarget* texture = faces[i];

		sourceRegion.left = 0;
		sourceRegion.right = width;
		sourceRegion.top = 0;
		sourceRegion.bottom = height;
		sourceRegion.front = 0;
		sourceRegion.back = 1;

		dc->CopySubresourceRegion(m_pTexture, D3D11CalcSubresource(mipSlice, i, m_MipMaps), 0, 0, 0, texture->GetTexture(),
			0, &sourceRegion);
	}
}
