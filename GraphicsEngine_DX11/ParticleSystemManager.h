#pragma once
#include <vector>
#include <queue>
#include <memory>
#include "Shared_RenderingData.h"

class DX11Core;
class ParticleSystem;
class ResourceManager;

/// <summary>
/// Particle System을 관리하는 매니저
/// 생성, 초기화 담당
/// 2022. 03. 17 정종영
/// </summary>
class ParticleSystemManager
{
public:
	ParticleSystemManager(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm);
	~ParticleSystemManager();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	std::vector<std::unique_ptr<ParticleSystem>> m_ParticleSystemVec;	// 게임 엔진에 현존하는 목록
	std::queue<ParticleSystem*> m_ParticleSystemQueue;					// 이번에 업데이트&렌더링 해야할 목록

public:
	// Game Engine으로부터 데이터의 포인터를 받고 Particle System클래스를 생성하고 그 데이터를 넣어준다.
	// 업데이트를 위해 벡터에 넣어준다.
	void InitializeParticleSystem(Shared_ParticleSystemData* pData);
	void Update(float dTime, float totalTime);
	void Draw(const EMath::Matrix& view, const EMath::Matrix& proj);
	void ReleaseParticleSystem(std::wstring name);
};

