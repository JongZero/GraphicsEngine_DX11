#include "MacroDefine.h"
#include "ParticleSystem.h"
#include "TextureLoader.h"
#include "Vertex.h"
#include "Effects.h"
#include "DX11Core.h"
#include "ErrChecker.h"
#include "ResourceManager.h"

ParticleSystem::ParticleSystem(std::shared_ptr<DX11Core> dx11Core, std::shared_ptr<ResourceManager> rm, Shared_ParticleSystemData* pData)
	: m_DX11Core(dx11Core), m_InitVB(0), m_DrawVB(0), m_StreamOutVB(0), m_TexArraySRV(0), m_RandomTexSRV(0),
	m_ResourceManager(rm), m_ParticleSystemData(pData)
{
	m_IsFirstRun = true;
	m_GameTime = 0.0f;
	m_TimeStep = 0.0f;
	m_Age = 0.0f;
}

ParticleSystem::~ParticleSystem()
{
	ReleaseCOM(m_InitVB);
	ReleaseCOM(m_DrawVB);
	ReleaseCOM(m_StreamOutVB);
}

void ParticleSystem::Initialize(UINT maxParticles)
{
	m_ParticleSystemData->m_MaxParticles = maxParticles;

	m_RandomTexSRV = m_ResourceManager->CreateRandomTexture1DSRV();

	BuildVB();
}

void ParticleSystem::Reset()
{
	m_IsFirstRun = true;
	m_Age = 0.0f;
}

void ParticleSystem::Update(float dt, float gameTime)
{
	m_GameTime = gameTime;
	m_TimeStep = dt;

	m_Age += dt;
}

void ParticleSystem::Draw(const EMath::Matrix& view, const EMath::Matrix& proj)
{
	EMath::Matrix _vp = view * proj;

	ID3D11DeviceContext* _dc = m_DX11Core->GetDC();
	ParticleEffect* _fx = Effects::FireFX;

	// ���ҽ� �Ŵ����κ��� ��ƼŬ �ؽ��ĸ� �����´�.
	if(m_TexArraySRV == nullptr)
		m_TexArraySRV = m_ResourceManager->GetParticle(m_ParticleSystemData->m_Name);

	// cb ����
	_fx->SetViewProj(_vp);
	_fx->SetGameTime(m_GameTime);
	_fx->SetTimeStep(m_TimeStep);
	_fx->SetEmitPosW(m_ParticleSystemData->m_EmitPos);
	_fx->SetEmitDirW(m_ParticleSystemData->m_EmitDir);
	_fx->SetTexArray(m_TexArraySRV);
	_fx->SetRandomTex(m_RandomTexSRV);

	// IA ����
	_dc->IASetInputLayout(InputLayouts::Particle);
	_dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(Vertex::Particle);
	UINT offset = 0;

	// ���� �����̸� �ʱ�ȭ�� ���� ���۸� ����ϰ�,
	// �׷��� ������ ������ ���� ����� ���� ���� ���۸� ����Ѵ�.
	if (m_IsFirstRun)
		_dc->IASetVertexBuffers(0, 1, &m_InitVB, &stride, &offset);
	else
		_dc->IASetVertexBuffers(0, 1, &m_DrawVB, &stride, &offset);

	// ���� ���� ����� ��Ʈ�� ��� ���� ������� �׷��� ���ڵ��� �����Ѵ�.
	// ���ŵ� ���ڵ��� ��Ʈ�� ����� ���ؼ� ��� ���� ���ۿ� ��ϵȴ�.
	_dc->SOSetTargets(1, &m_StreamOutVB, &offset);

	D3DX11_TECHNIQUE_DESC techDesc;
	_fx->StreamOutTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		_fx->StreamOutTech->GetPassByIndex(p)->Apply(0, _dc);

		if (m_IsFirstRun)
		{
			_dc->Draw(1, 0);
			m_IsFirstRun = false;
		}
		else
		{
			_dc->DrawAuto();
		}
	}

	// ��Ʈ�� ���� �н��� ������. ���� ���۸� �����.
	ID3D11Buffer* bufferArray[1] = { 0 };
	_dc->SOSetTargets(1, bufferArray, &offset);

	// ���� ���۵��� �¹ٲ۴� (����)
	std::swap(m_DrawVB, m_StreamOutVB);

	// ��� ��Ʈ�� ��µ�, ���ŵ� ���� �ý����� ȭ�鿡 �׸���.
	_dc->IASetVertexBuffers(0, 1, &m_DrawVB, &stride, &offset);

	_fx->DrawTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		_fx->DrawTech->GetPassByIndex(p)->Apply(0, _dc);

		_dc->DrawAuto();
	}
}

void ParticleSystem::BuildVB()
{
	ID3D11Device* device = m_DX11Core->GetDevice();

	// Create the buffer to kick-off the particle system.

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(Vertex::Particle) * 1;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	Vertex::Particle p;
	ZeroMemory(&p, sizeof(Vertex::Particle));
	p.Age = 0.0f;
	p.Type = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;

	HR(device->CreateBuffer(&vbd, &vinitData, &m_InitVB));

	// Create the ping-pong buffers for stream-out and drawing.
	vbd.ByteWidth = sizeof(Vertex::Particle) * (UINT)m_ParticleSystemData->m_MaxParticles;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

	HR(device->CreateBuffer(&vbd, 0, &m_DrawVB));
	HR(device->CreateBuffer(&vbd, 0, &m_StreamOutVB));
}
