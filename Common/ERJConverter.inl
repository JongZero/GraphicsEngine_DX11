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
	// T타입의 크기만큼 그대로 ostream에 write한다.
	if (!os.write((const char*)&v, sizeof(T)))
	{
		// write 실패한 경우엔 오류 표시
		std::cout << std::string{ "Writing type '" } + typeid(T).name() + "' Failed\n";
	}
}

template <class T>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const typename std::vector<T>& v)
{
	// 먼저 vector의 크기를 저장
	WriteToERJ<uint32_t>(os, v.size());

	// 그리고 각 요소를 하나씩 저장
	for (const auto& e : v)
	{
		WriteToERJ(os, e);
	}
}

template<class T1, class T2>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const typename std::map<T1, T2>& v)
{
	// map의 크기를 저장
	WriteToERJ<uint32_t>(os, v.size());

	// 각 요소를 하나씩 저장
	for (const auto& p : v)
	{
		WriteToERJ(os, p);
	}
}

template<class T1, class T2>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const typename std::unordered_map<T1, T2>& v)
{
	// map의 크기를 저장
	WriteToERJ<uint32_t>(os, v.size());

	// 각 요소를 하나씩 저장
	for (const auto& p : v)
	{
		WriteToERJ(os, p);
	}
}

template<class _Ty>
inline void ERJConverter::WriteToERJ_OneByOne(std::ostream& os, const typename std::basic_string<_Ty>& v)
{
	// string 크기를 저장
	WriteToERJ<uint32_t>(os, v.size());

	// 그리고 각 요소를 하나씩 저장
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
	// 먼저 vector의 크기를 읽어옴
	v.resize(ReadFromERJ<uint32_t>(is));

	// 그리고 각 요소를 하나씩 읽음
	for (const auto& e : v)
	{
		ReadFromERJ(is, e);
	}
}

template<class T1, class T2>
inline void ERJConverter::ReadFromERJ_OneByOne(std::istream& is, typename std::map<T1, T2>& v)
{
	// 먼저 map의 크기를 읽는다.
	size_t len = ReadFromERJ<uint32_t>(is);
	v.clear();

	// 요소를 하나씩 읽는다.
	for (size_t i = 0; i < len; ++i)
	{
		v.emplace(ReadFromERJ<std::pair<T1, T2>>(is));
	}
}

template<class T1, class T2>
inline void ERJConverter::ReadFromERJ_OneByOne(std::istream& is, typename std::unordered_map<T1, T2>& v)
{
	// 먼저 map의 크기를 읽는다.
	size_t len = ReadFromERJ<uint32_t>(is);
	v.clear();

	// 요소를 하나씩 읽는다.
	for (size_t i = 0; i < len; ++i)
	{
		v.emplace(ReadFromERJ<std::pair<T1, T2>>(is));
	}
}

template<class T>
inline void ERJConverter::ReadFromERJ_OneByOne(std::istream& is, typename std::basic_string<T>& v)
{
	// string의 크기를 읽음
	v.resize(ReadFromERJ<uint32_t>(is));

	// 그리고 각 요소를 하나씩 읽음
	for (const auto& e : v)
	{
		ReadFromERJ(is, e);
	}
}