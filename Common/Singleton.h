#pragma once

/// <summary>
/// 싱글톤 클래스를 생성할 때 이 클래스를 상속받아서 사용
/// 2021. 10. 22. 정종영
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
class Singleton
{
protected:
	static T* m_Instance;

protected:
	Singleton() { }
	~Singleton() { }

public:
	static T* GetInstance();
	void DeleteInstance();
};

template <typename T>
T* Singleton<T>::m_Instance = nullptr;

template <typename T>
inline T* Singleton<T>::GetInstance()
{
	if (!m_Instance)
	{
		m_Instance = new T();
	}

	assert(m_Instance != nullptr);

	return m_Instance;
}

template<typename T>
inline void Singleton<T>::DeleteInstance()
{
	assert(m_Instance != nullptr);

	if (m_Instance)
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
}