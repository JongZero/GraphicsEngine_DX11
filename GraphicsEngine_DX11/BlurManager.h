#pragma once
#include <memory>
#include "Effects.h"
#include "EMath.h"

class DX11Core;
class RenderTarget;
class Scaler;
struct Shared_RenderingData;

/// <summary>
/// ��, ���, ��Ǻ����� ���� �Ŵ���
/// 2021. 07. ������
/// </summary>
class BlurManager
{
public:
	BlurManager(std::shared_ptr<DX11Core> dx11Core);
	~BlurManager();

public:
	bool IsActive;			// �Ŵ��� �ٱ����� Active�� ��Ʈ���Ѵ�.

private:
	std::shared_ptr<DX11Core> m_DX11Core;
	ID3DX11EffectTechnique* m_pActiveTech;

	UINT m_Width;
	UINT m_Height;
	float m_BloomSize;

	std::unique_ptr<RenderTarget> m_BlurSetter;
	std::unique_ptr<RenderTarget> m_BlurHelper;

	std::unique_ptr<RenderTarget> m_BloomDownScale4x4_0;	// 4x4 DownScale Output
	std::unique_ptr<RenderTarget> m_BloomCurve_1;			// Bloom Curve Output
	std::unique_ptr<RenderTarget> m_BloomDownScale6x6_2;	// 6x6 DownScale Output
	std::unique_ptr<RenderTarget> m_BloomDownScale6x6_3;	// 6x6 DownScale Output
	std::unique_ptr<RenderTarget> m_BloomUpScale6x6_4;		// 6x6 UpScale Output
	std::unique_ptr<RenderTarget> m_BloomUpScale6x6_5;		// 6x6 UpScale Output
	std::unique_ptr<RenderTarget> m_BloomUpScale4x4_6;		// 4x4 UpScale Output

	std::unique_ptr<RenderTarget> m_BloomOriginal;			// Original (����)
	std::unique_ptr<RenderTarget> m_BloomAccumulate_0;		// Accumulate Output 0
	std::unique_ptr<RenderTarget> m_BloomAccumulate_1;		// Accumulate Output 1
	std::unique_ptr<RenderTarget> m_BloomAccumulate_2;		// Accumulate Output 2
	std::unique_ptr<RenderTarget> m_BloomAccumulate_3;		// Accumulate Output 3

	EMath::Matrix m_ViewProj_Prev;

public:
	void OnResize(UINT width, UINT height);

public:
	void Blur(RenderTarget* input, float weights = 0.0f, int blurCount = 1);		// input�� �����س��� blur���� �ʱ� ������
																					// input�� �ٷ� blur�� �Ѵ�. (������ ��������)
																					// ���� ����, input = output�̴�.

	void SetGaussianWeights(float sigma);
	void MotionBlur(RenderTarget* output, Shared_RenderingData* dataForRender, RenderTarget* depthMap, RenderTarget* final);
	void Bloom(Scaler* scaler, RenderTarget* output, RenderTarget* input);

private:
	void CopyOriginalRenderTarget(RenderTarget* output, RenderTarget* input);
	void BloomCurve(RenderTarget* output, RenderTarget* input);
	void Accumulate(RenderTarget* output, RenderTarget* original, RenderTarget* input);
	void Accumulate_Per(RenderTarget* output, RenderTarget* input1, RenderTarget* input2);
};