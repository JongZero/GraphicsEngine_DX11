#pragma once
#include <memory>

class DX11Core;
class ResourceManager;
class RenderTarget;
struct Shared_RenderingData;

/// Emissive Map (�߱� ��)�� ����ϴ� Ŭ����
/// 2022. 01. 24 ������
class EmissiveMapper
{
public:
	EmissiveMapper(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	~EmissiveMapper();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;

public:
	void PrePass(RenderTarget* output, Shared_RenderingData* dataForRender);
	void Mix(RenderTarget* output, RenderTarget* lastRenderTarget, RenderTarget* preEmissiveOutput);
};
