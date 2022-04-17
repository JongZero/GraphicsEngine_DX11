#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "EMath.h"
#include "ParserData.h"

/// <summary>
/// FBX 파일을 파싱한 데이터를 ERF 파일로 변환하는 클래스
/// 2022. 02. 17 정종영
/// </summary>
class ERJConverter
{
public:
	ERJConverter() {};
	~ERJConverter() {};

	template <class T> inline void WriteToERJ(std::ostream& os, const T& v);
	template <class T> inline void ReadFromERJ(std::istream& is, T& v);
	template <class T> inline T ReadFromERJ(std::istream& is);

private:
	/// Write
	template <class T> inline typename void WriteToERJ_OneByOne(std::ostream& os, const T& v);

	// 컨테이너
	template <class T> inline void WriteToERJ_OneByOne(std::ostream& os, const typename std::vector<T>& v);
	template<class T1, class T2> inline void WriteToERJ_OneByOne(std::ostream& os, const typename std::map<T1, T2>& v);
	template<class T1, class T2> inline void WriteToERJ_OneByOne(std::ostream& os, const typename std::unordered_map<T1, T2>& v);
	template<class _Ty> inline void WriteToERJ_OneByOne(std::ostream& os, const typename std::basic_string<_Ty>& v);

private:
	/// Read
	template<class T> inline typename void ReadFromERJ_OneByOne(std::istream& is, T& v);

	// 컨테이너
	template<class T> inline void ReadFromERJ_OneByOne(std::istream& is, typename std::vector<T>& v);
	template<class T1, class T2> inline void ReadFromERJ_OneByOne(std::istream& is, typename std::map<T1, T2>& v);
	template<class T1, class T2> inline void ReadFromERJ_OneByOne(std::istream& is, typename std::unordered_map<T1, T2>& v);
	template<class T> inline void ReadFromERJ_OneByOne(std::istream& is, typename std::basic_string<T>& v);
};

#include "ERJConverter.inl"