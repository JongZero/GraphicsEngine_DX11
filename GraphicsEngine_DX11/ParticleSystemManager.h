#pragma once
#include <vector>
#include <queue>
#include <memory>
#include "Shared_RenderingData.h"

class DX11Core;
class ParticleSystem;
class ResourceManager;

/// <summary>
/// Particle System�� �����ϴ� �Ŵ���
/// ����, �ʱ�ȭ ���
/// 2022. 03. 17 ������
/// </summary>
class ParticleSystemManager
{
public:
	ParticleSystemManager(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	~ParticleSystemManager();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	std::vector<std::unique_ptr<ParticleSystem>> m_ParticleSystemVec;	// ���� ������ �����ϴ� ���
	std::queue<ParticleSystem*> m_ParticleSystemQueue;					// �̹��� ������Ʈ&������ �ؾ��� ���

public:
	// Game Engine���κ��� �������� �����͸� �ް� Particle SystemŬ������ �����ϰ� �� �����͸� �־��ش�.
	// ������Ʈ�� ���� ���Ϳ� �־��ش�.
	void InitializeParticleSystem(Shared_ParticleSystemData* pData);
	void Update(float dTime, float totalTime);
	void Draw(const EMath::Matrix& view, const EMath::Matrix& proj);
	void ReleaseParticleSystem(std::wstring name);
};

