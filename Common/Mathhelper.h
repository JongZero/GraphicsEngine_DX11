//***************************************************************************************
// MathHelper.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Helper math class.
//***************************************************************************************

#ifndef MATHHELPER_H
#define MATHHELPER_H

#include <Windows.h>
#include <random>
#include "EMath.h"

class MathHelper
{
public:
	// Returns random float in [0, 1).
	// [0,1) 구간의 부동소수점 난수를 돌려준다.
	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	// [a,b) 구간의 부동소수점 난수를 돌려준다.
	static float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
	 
	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b-a)*t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

	// Returns the polar angle of the point (x,y) in [0, 2*PI).
	static float AngleFromXY(float x, float y);

	static EMath::Matrix InverseTranspose(const EMath::Matrix& M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
		EMath::Matrix A(M);
		A.m[3][0] = 0;
		A.m[3][1] = 0;
		A.m[3][2] = 0;
		A.m[3][3] = 1;

		EMath::Matrix invA = A.Invert();
		return invA.Transpose();
	}

	static EMath::Vector3 RandUnitVec3();
	//static XMVECTOR RandHemisphereUnitVec3(XMVECTOR n);

	static const float Infinity;
	static const float Pi;

	//static XMFLOAT3 XMFloat3Sum(const XMFLOAT3& a, const XMFLOAT3& b)
	//{
	//	XMFLOAT3 result;
	//
	//	result.x = a.x + b.x;
	//	result.y = a.y + b.y;
	//	result.z = a.z + b.z;
	//
	//	return result;
	//}
};

#endif // MATHHELPER_H