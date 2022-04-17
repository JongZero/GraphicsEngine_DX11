#pragma once
#include "EMath.h"
#include "fbxsdk.h"

/// Fbx의 Vector, Matrix들을 EMath의 Vector, Matrix로 변환하여 반환해준다.
/// 2021. 11. 16 정종영
class MathConverter_Fbx
{
public:
	static EMath::Matrix FbxMatrix_To_EMathMatrix(FbxMatrix f, bool isReverse = false);
	static EMath::Vector4 FbxVector4_To_EMathVector4(FbxVector4 f);
	static EMath::Vector3 FbxVector4_To_EMathVector3(FbxVector4 f);
};

