#include "MathConverter_Fbx.h"

EMath::Matrix MathConverter_Fbx::FbxMatrix_To_EMathMatrix(FbxMatrix f, bool isReverse)
{
	EMath::Matrix m;
	
	isReverse = true;

	if (isReverse == false)
	{
		m._11 = static_cast<float>(f.GetRow(0)[0]);
		m._12 = static_cast<float>(f.GetRow(0)[1]);
		m._13 = static_cast<float>(f.GetRow(0)[2]);
		m._14 = static_cast<float>(f.GetRow(0)[3]);

		m._21 = static_cast<float>(f.GetRow(1)[0]);
		m._22 = static_cast<float>(f.GetRow(1)[1]);
		m._23 = static_cast<float>(f.GetRow(1)[2]);
		m._24 = static_cast<float>(f.GetRow(1)[3]);

		m._31 = static_cast<float>(f.GetRow(2)[0]);
		m._32 = static_cast<float>(f.GetRow(2)[1]);
		m._33 = static_cast<float>(f.GetRow(2)[2]);
		m._34 = static_cast<float>(f.GetRow(2)[3]);

		m._41 = static_cast<float>(f.GetRow(3)[0]);
		m._42 = static_cast<float>(f.GetRow(3)[1]);
		m._43 = static_cast<float>(f.GetRow(3)[2]);
		m._44 = static_cast<float>(f.GetRow(3)[3]);
	}
	// 가운데 행을 서로 뒤집어줌
	else
	{
		m._11 = f.mData[0].mData[0];		m._13 = f.mData[0].mData[1];	m._12 = f.mData[0].mData[2];	m._14 = f.mData[0].mData[3];
		m._21 = f.mData[2].mData[0];		m._23 = f.mData[2].mData[1];	m._22 = f.mData[2].mData[2];	m._24 = f.mData[2].mData[3];
		m._31 = f.mData[1].mData[0];		m._33 = f.mData[1].mData[1];	m._32 = f.mData[1].mData[2];	m._34 = f.mData[1].mData[3];
		m._41 = f.mData[3].mData[0];		m._43 = f.mData[3].mData[1];	m._42 = f.mData[3].mData[2];	m._44 = f.mData[3].mData[3];
	}
	
	return m;
}

EMath::Vector4 MathConverter_Fbx::FbxVector4_To_EMathVector4(FbxVector4 f)
{
	EMath::Vector4 v;
	v.x = f[0];
	v.y = f[1];
	v.z = f[2];
	v.w = f[3];
	return v;
}

EMath::Vector3 MathConverter_Fbx::FbxVector4_To_EMathVector3(FbxVector4 f)
{
	EMath::Vector3 v;
	v.x = f[0];
	v.y = f[1];
	v.z = f[2];
	return v;
}