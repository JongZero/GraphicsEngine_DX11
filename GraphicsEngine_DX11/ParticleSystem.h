#pragma once
#include <memory>
#include <d3d11.h>
#include <string>
#include "EMath.h"
#include "Shared_RenderingData.h"

class DX11Core;
class ResourceManager;

/// <summary>
/// 그래픽스 엔진 내부의 파티클 시스템
/// </summary>
class ParticleSystem
{
public:
	ParticleSystem(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm, Shared_ParticleSystemData* pData);
	~ParticleSystem();

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	std::shared_ptr<ResourceManager> m_ResourceManager;
	Shared_ParticleSystemData* m_ParticleSystemData;

	bool m_IsFirstRun;		// 첫 프레임인지?

	float m_GameTime;
	float m_TimeStep;
	float m_Age;

	ID3D11Buffer* m_InitVB;
	ID3D11Buffer* m_DrawVB;
	ID3D11Buffer* m_StreamOutVB;

	ID3D11ShaderResourceView* m_TexArraySRV;
	ID3D11ShaderResourceView* m_RandomTexSRV;

public:
	Shared_ParticleSystemData* GetParticleSystemData() { return m_ParticleSystemData; }

public:
	void Initialize(UINT maxParticles);
	void Reset();
	void Update(float dt, float gameTime);
	void Draw(const EMath::Matrix& view, const EMath::Matrix& proj);

private:
	void BuildVB();
};

