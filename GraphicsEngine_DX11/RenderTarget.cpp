#include "RenderTarget.h"
#include "RenderTargetDrawer.h"
#include "MacroDefine.h"
#include "ErrChecker.h"
#include "DebugManager.h"
#include "ClientSize.h"

RenderTarget::RenderTarget(int width, int height, std::wstring name, RenderTargetOrder order)
	: m_Name(name), m_Order(order), m_World(EMath::Matrix::Identity), m_Scale(EMath::Vector2(1,1)), m_Position(EMath::Vector2(0,0)),
	m_Width(0), m_Height(0)
{
	SetWorld();
}

RenderTarget::~RenderTarget()
{
	m_ShaderResourceView = nullptr;
	m_RenderTargetView = nullptr;
	m_UnorderedAccessView = nullptr;
}

void RenderTarget::SetWorld()
{
	m_World = EMath::Matrix::CreateScale(m_Scale.x, m_Scale.y, 0) * EMath::Matrix::CreateTranslation(m_Position.x, m_Position.y, 0);
}

void RenderTarget::OnResize(ID3D11Device* device, int width, int height)
{
	// �ٲ����� ����� ���ݰ� �Ȱ��ٸ� ���� �ڿ� �����ϰ� ������ �ʿ䰡 ����.
	if (m_Width == width && m_Height == height)
		return;

	m_Width = width;
	m_Height = height;

	// We call this function everytime the window is resized so that the render target is a quarter
	// the client area dimensions.  So Release the previous views before we create new ones.
	m_ShaderResourceView = nullptr;
	m_RenderTargetView = nullptr;
	m_UnorderedAccessView = nullptr;

	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	HR(device->CreateTexture2D(&texDesc, 0, m_Texture.GetAddressOf()));

	// Null description means to create a view to all mipmap levels using 
	// the format the texture was created with.
	HR(device->CreateRenderTargetView(m_Texture.Get(), 0, &m_RenderTargetView));

	HR(device->CreateShaderResourceView(m_Texture.Get(), 0, &m_ShaderResourceView));
	HR(device->CreateUnorderedAccessView(m_Texture.Get(), 0, &m_UnorderedAccessView));
}

void RenderTarget::Draw(ID3D11DeviceContext* dc, ID3DX11EffectTechnique* tech /*= Effects::DebugTexFX->ViewArgbTech*/)
{
	RenderTargetDrawer::DrawRenderTarget(dc, this, tech);

	float posX = m_Position.x;
	float posY = -m_Position.y;		// y ��ǥ�� ������ �ݴ����� ���

	// projection ��ȯ�� ���� ��ȯ�� ��ǥ�� -1.0f ~ 1.0f ������ ��
	// -1.0f ~ 1.0f ������ 0.0f ~ 1.0f (����) ������ ����
	posX = (posX + 1.0f - m_Scale.x) / 2.0f ;
	posY = (posY + 1.0f - m_Scale.y) / 2.0f ;

	// viewport ũ��� ������ ����Ͽ� client ��ǥ�� ��´�.
	posX = ClientSize::GetWidth() * (posX);
	posY = ClientSize::GetHeight() * (posY);

	// �ڽ��� �̸� ���
	DebugManager::GetInstance()->AddText(TextInfo(
		EMath::Vector2(posX, posY),
		D2D1::ColorF::Black, (TCHAR*)m_Name.c_str()));
}
