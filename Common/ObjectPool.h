#pragma once
/// <summary>
/// [2021-11-17][�ֹΰ�]
/// ���� �����忡 ������ ������Ʈ Ǯ
/// </summary>
template <typename T>
class ObjectPool
{
#pragma region CObject
private:
	template<typename T>
	class CObject
	{
	public:
		CObject()
		{
			object = T();
			isUsable = true;
		}
		~CObject()
		{
		}

	public:
		T			object;			// ������Ʈ ����
		bool		isUsable;		// ��� ���� ����(���� ����)
	};
#pragma endregion

public:
	ObjectPool(int num = 100, bool isExpand = false);
	~ObjectPool();

public:
	// ���ǰ� ���� ���� ������Ʈ�� Ž���Ͽ� �������� �Լ�
	T* GetUsableObject();
	// ����� ���� ������Ʈ�� Ǯ�� �����ִ� �Լ�
	void ReturnObject(T* object);

private:
	// num����ŭ ������Ʈ Ǯ�� ������Ʈ�� �����ϴ� �Լ�
	void CreateObjectPool(int num);

private:
	mutable std::shared_mutex		m_SharedMutex;
	std::vector<CObject<T>>			m_ObjectVec;
	bool							m_isExpand;			// ��� ������ ������Ʈ�� ���� �� �߰� Ȯ�� ����
};

template<typename T>
inline ObjectPool<T>::ObjectPool(int num, bool isExpand)
	: m_isExpand(isExpand)
{
	CreateObjectPool(num);
}

template<typename T>
inline ObjectPool<T>::~ObjectPool()
{
	//// ������Ʈ Ǯ ���� ��� ������Ʈ ã�Ƽ� �����ϱ�
	//for (CObject<T>*& obj : m_ObjectVec)
	//{
	//	T* _tempObject = obj->object;
	//	_tempObject->~T();
	//}
}

template<typename T>
inline T* ObjectPool<T>::GetUsableObject()
{
	std::unique_lock<std::shared_mutex> _ulk(m_SharedMutex);

	T* _usableObject = nullptr;

	do
	{
		// ������Ʈ Ǯ���� ��� ������ ������Ʈ ã��
		for (CObject<T>& obj : m_ObjectVec)
		{
			if (obj.isUsable == true)
			{
				_usableObject = &(obj.object);
				obj.isUsable = false;
				break;
			}
		}

		// Ȯ�� �ɼ��� ���������� ������Ʈ Ǯ �ø���
		if (m_isExpand == true)
		{
			if (_usableObject == nullptr)
			{
				CreateObjectPool(m_ObjectVec.size());
			}
		}
		else
		{
			break;
		}

	} while (_usableObject == nullptr);

	return _usableObject;
}

template<typename T>
inline void ObjectPool<T>::ReturnObject(T* object)
{
	// ������Ʈ Ǯ���� �ش� ������Ʈ ã�Ƽ� �ٽ� �غ� ��Ű��
	for (CObject<T>& obj : m_ObjectVec)
	{
		if (&(obj.object) == object)
		{
			obj.isUsable = true;
		}
	}
}

template<typename T>
inline void ObjectPool<T>::CreateObjectPool(int num)
{
	for (int i = 0; i < num; i++)
	{
		m_ObjectVec.push_back(CObject<T>());
	}
}
