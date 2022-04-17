#pragma once

template <class T>
inline void ERJConverter::WriteToERJ(std::ostream& os, const T& v)
{
	WriteToERJ_OneByOne(os, v);
}

template <class T>
inline void ERJConverter::ReadFromERJ(std::istream& is, T& v)
{
	ReadFromERJ_OneByOne(is, v);
}

template <class T>
inline T ERJConverter::ReadFromERJ(std::istream& is)
{
	T v;
	ReadFromERJ_OneByOne(is, v);
	return v;
}

template <class T>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const T& v)
{
	// TŸ���� ũ�⸸ŭ �״�� ostream�� write�Ѵ�.
	if (!os.write((const char*)&v, sizeof(T)))
	{
		// write ������ ��쿣 ���� ǥ��
		std::cout << std::string{ "Writing type '" } + typeid(T).name() + "' Failed\n";
	}
}

template <class T>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const typename std::vector<T>& v)
{
	// ���� vector�� ũ�⸦ ����
	WriteToERJ<uint32_t>(os, v.size());

	// �׸��� �� ��Ҹ� �ϳ��� ����
	for (const auto& e : v)
	{
		WriteToERJ(os, e);
	}
}

template<class T1, class T2>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const typename std::map<T1, T2>& v)
{
	// map�� ũ�⸦ ����
	WriteToERJ<uint32_t>(os, v.size());

	// �� ��Ҹ� �ϳ��� ����
	for (const auto& p : v)
	{
		WriteToERJ(os, p);
	}
}

template<class T1, class T2>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const typename std::unordered_map<T1, T2>& v)
{
	// map�� ũ�⸦ ����
	WriteToERJ<uint32_t>(os, v.size());

	// �� ��Ҹ� �ϳ��� ����
	for (const auto& p : v)
	{
		WriteToERJ(os, p);
	}
}

template<class _Ty>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const typename std::basic_string<_Ty>& v)
{
	// string ũ�⸦ ����
	WriteToERJ<uint32_t>(os, v.size());

	// �׸��� �� ��Ҹ� �ϳ��� ����
	for (const auto& e : v)
	{
		WriteToERJ(os, e);
	}
}

template<class T>
inline typename void ERJConverter::ReadFromERJ_OneByOne(std::istream& is, T& v)
{
	if (!is.read((char*)&v, sizeof(T)))
	{
		std::cout << std::string{ "reading type '" } + typeid(T).name() + "' failed\n";
	}
}

template<class T>
inline void ERJConverter::ReadFromERJ_OneByOne(std::istream& is, typename std::vector<T>& v)
{
	// ���� vector�� ũ�⸦ �о��
	v.resize(ReadFromERJ<uint32_t>(is));

	// �׸��� �� ��Ҹ� �ϳ��� ����
	for (const auto& e : v)
	{
		ReadFromERJ(is, e);
	}
}

template<class T1, class T2>
inline void ERJConverter::ReadFromERJ_OneByOne(std::istream& is, typename std::map<T1, T2>& v)
{
	// ���� map�� ũ�⸦ �д´�.
	size_t len = ReadFromERJ<uint32_t>(is);
	v.clear();

	// ��Ҹ� �ϳ��� �д´�.
	for (size_t i = 0; i < len; ++i)
	{
		v.emplace(ReadFromERJ<std::pair<T1, T2>>(is));
	}
}

template<class T1, class T2>
inline void ERJConverter::ReadFromERJ_OneByOne(std::istream& is, typename std::unordered_map<T1, T2>& v)
{
	// ���� map�� ũ�⸦ �д´�.
	size_t len = ReadFromERJ<uint32_t>(is);
	v.clear();

	// ��Ҹ� �ϳ��� �д´�.
	for (size_t i = 0; i < len; ++i)
	{
		v.emplace(ReadFromERJ<std::pair<T1, T2>>(is));
	}
}

template<class T>
inline void ERJConverter::ReadFromERJ_OneByOne(std::istream& is, typename std::basic_string<T>& v)
{
	// string�� ũ�⸦ ����
	v.resize(ReadFromERJ<uint32_t>(is));

	// �׸��� �� ��Ҹ� �ϳ��� ����
	for (const auto& e : v)
	{
		ReadFromERJ(is, e);
	}
}