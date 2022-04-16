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
	// 생성 후 벡터에 넣는다.
	auto ps = std::make_unique<ParticleSystem>(m_DX11Core, m_ResourceManager, pData);
	ps->Initialize((UINT)pData->m_MaxParticles);
	m_ParticleSystemVec.push_back(std::move(ps));
}

void ParticleSystemManager::Update(float dTime, float totalTime)
{
	// 활성화되어있는지 확인하고,
	// Queue에 먼저 담는다.
	for (const auto& it : m_ParticleSystemVec)
	{
		// 활성화되어 있는지
		if (it->GetParticleSystemData()->m_IsActive)
		{
			// 리셋 해야한다면
			if (it->GetParticleSystemData()->m_IsReset)
			{
				it->Reset();
				it->GetParticleSystemData()->m_IsReset = false;
			}

			// 활성화되어있으면 업데이트
			it->Update(dTime, totalTime);

			// 후에 큐에 넣어줌
			m_ParticleSystemQueue.push(it.get());
		}
	}
}

void ParticleSystemManager::Draw(const EMath::Matrix& view, const EMath::Matrix&  proj)
{
	// Queue를 순회하며 Draw 시킨다.
	while (!m_ParticleSystemQueue.empty())
	{
		ParticleSystem* ps = m_ParticleSystemQueue.front();
		ps->Draw(view, proj);
		m_ParticleSystemQueue.pop();
	}

	// 블렌딩을 꺼준다.
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
