#pragma once

//---------------------------------------------------------------------------------------
// Simple d3d error checker
//---------------------------------------------------------------------------------------
#include "dxerr.h"

#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)                                              \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			DXTrace(__FILEW__, (DWORD)__LINE__, hr, L#x, true);\
		}                                                      \
	}
#endif

#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 