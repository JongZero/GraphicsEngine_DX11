#pragma once

/// <summary>
/// �ð��� ���, ��ŸŸ���� ���ϱ����� ���δ�.
/// 2021. 10. 22. ������
/// </summary>
class Timer
{
public:
	Timer();
	~Timer();
	
private:
	double m_SecondsPerCount;
	double m_DeltaTime;

	__int64 m_PrevTime;
	__int64 m_CurrTime;

	// ��ü �ð�(Total Time)�� ���ϱ� ���� ����
	__int64 m_BaseTime;
	__int64 m_PausedTime;		// Ÿ�̸Ӱ� �Ͻ� ������ ���� ����ؼ� ������
	__int64 m_StopTime;			// Ÿ�̸Ӱ� �Ͻ� ������ ������ �ð�

	bool m_IsStopped;

public:
	float TotalTime() const;		// �� ����, Reset�� ȣ��� ���� �帥 �ð����� �Ͻ� ������ �ð��� ������ �ð��� �����ش�.
	float DeltaTime() const { return (float)m_DeltaTime; };	// �� ����

	void Reset();	// �޽��� ���� ������ ȣ���ؾ� ��
	void Stop();	// �Ͻ����� �� ȣ���ؾ� ��
	void Start();	// Ÿ�̸� ����,�簳 �� ȣ���ؾ� ��
	void Tick();	// �� �����Ӹ��� ȣ���ؾ� ��
};

