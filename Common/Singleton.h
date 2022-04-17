#pragma once

/// <summary>
/// �̱��� Ŭ������ ������ �� �� Ŭ������ ��ӹ޾Ƽ� ���
/// 2021. 10. 22. ������
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