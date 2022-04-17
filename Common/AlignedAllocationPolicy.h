#pragma once
#include <malloc.h>

// 16 바이트 정렬을 위해
template<size_t T>
class AlignedAllocationPolicy
{
public:
	static void* operator new(size_t size) { return _aligned_malloc(size, T); }
	static void operator delete(void* memory) { _aligned_free(memory); }
};
