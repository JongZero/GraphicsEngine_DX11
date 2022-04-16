#include "ParticleSystemManager.h"
#include "ParticleSystem.h"
#include "DX11Core.h"

ParticleSystemManager::ParticleSystemManager(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm)
	: m_DX11Core(dx11Core), m_ResourceManager(rm)
{

}

ParticleSystemManager::~ParticleSystemManager()
{

}

void ParticleSystemManager::InitializeParticleSystem(Shared_ParticleSystemData* pData)
{
	// ���� �� ���Ϳ� �ִ´�.
	auto ps = std::make_unique<ParticleSystem>(m_DX11Core, m_ResourceManager, pData);
	ps->Initialize((UINT)pData->m_MaxParticles);
	m_ParticleSystemVec.push_back(std::move(ps));
}

void ParticleSystemManager::Update(float dTime, float totalTime)
{
	// Ȱ��ȭ�Ǿ��ִ��� Ȯ���ϰ�,
	// Queue�� ���� ��´�.
	for (const auto& it : m_ParticleSystemVec)
	{
		// Ȱ��ȭ�Ǿ� �ִ���
		if (it->GetParticleSystemData()->m_IsActive)
		{
			// ���� �ؾ��Ѵٸ�
			if (it->GetParticleSystemData()->m_IsReset)
			{
				it->Reset();
				it->GetParticleSystemData()->m_IsReset = false;
			}

			// Ȱ��ȭ�Ǿ������� ������Ʈ
			it->Update(dTime, totalTime);

			// �Ŀ� ť�� �־���
			m_ParticleSystemQueue.push(it.get());
		}
	}
}

void ParticleSystemManager::Draw(const EMath::Matrix& view, const EMath::Matrix&  proj)
{
	// Queue�� ��ȸ�ϸ� Draw ��Ų��.
	while (!m_ParticleSystemQueue.empty())
	{
		ParticleSystem* ps = m_ParticleSystemQueue.front();
		ps->Draw(view, proj);
		m_ParticleSystemQueue.pop();
	}

	// ������ ���ش�.
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_DX11Core->GetDC()->OMSetBlendState(0, blendFactor, 0xffffffff);
}

void ParticleSystemManager::ReleaseParticleSystem(std::wstring name)
{
	for (int i = 0; i < m_ParticleSystemVec.size(); i++)
	{
		if (wcscmp(m_ParticleSystemVec[i]->GetParticleSystemData()->m_Name, name.c_str()))
		{
			m_ParticleSystemVec.erase(m_ParticleSystemVec.begin() + i);
			break;
		}
	}
}
