#pragma once

/// <summary>
/// 시간을 재고, 델타타임을 구하기위해 쓰인다.
/// 2021. 10. 22. 정종영
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

	// 전체 시간(Total Time)을 구하기 위한 변수
	__int64 m_BaseTime;
	__int64 m_PausedTime;		// 타이머가 일시 정지된 동안 계속해서 누적됨
	__int64 m_StopTime;			// 타이머가 일시 정지된 시점의 시간

	bool m_IsStopped;

public:
	float TotalTime() const;		// 초 단위, Reset이 호출된 이후 흐른 시간에서 일시 정지된 시간을 제외한 시간을 돌려준다.
	float DeltaTime() const { return (float)m_DeltaTime; };	// 초 단위

	void Reset();	// 메시지 루프 이전에 호출해야 함
	void Stop();	// 일시정지 시 호출해야 함
	void Start();	// 타이머 시작,재개 시 호출해야 함
	void Tick();	// 매 프레임마다 호출해야 함
};

