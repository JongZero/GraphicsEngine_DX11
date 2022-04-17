#pragma once
#include "EMath.h"
#include "fbxsdk.h"

/// Fbx�� Vector, Matrix���� EMath�� Vector, Matrix�� ��ȯ�Ͽ� ��ȯ���ش�.
/// 2021. 11. 16 ������
class MathConverter_Fbx
{
public:
	static EMath::Matrix FbxMatrix_To_EMathMatrix(FbxMatrix f, bool isReverse = false);
	static EMath::Vector4 FbxVector4_To_EMathVector4(FbxVector4 f);
	static EMath::Vector3 FbxVector4_To_EMathVector3(FbxVector4 f);
};

