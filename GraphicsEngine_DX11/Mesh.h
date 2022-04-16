#ifndef MESH_H
#define MESH_H

#include <string>
#include <windows.h>
#include "EMath.h"

/// <summary>
/// 메쉬 하나의 데이터들
/// </summary>
struct Mesh
{
	UINT IndexCount = 0;
	UINT StartIndex = 0;
	UINT StartVertex = 0;

	std::string NodeName;
	std::string NodeParentName;

	EMath::Matrix LocalTM = EMath::Matrix::Identity;
	EMath::Matrix WorldTM = EMath::Matrix::Identity;

	EMath::Matrix NodeTM = EMath::Matrix::Identity;
	EMath::Matrix NodeInverseTM = EMath::Matrix::Identity;

	Mesh* Parent = nullptr;
	UINT ParentCount = 0;

	// 애니메이션 정보
	bool IsAnimated = false;				// 애니메이션이 있다고 하면 노드 네임을 검사해서 애니메이션 데이터와 매칭함
	EMath::Matrix AnimationTM = EMath::Matrix::Identity;	// 현재 프레임의 애니메이션 TM

	// 스키닝 정보
	bool IsBone = false;
	bool IsSkinnedMesh = false;
	UINT BoneIndex = 0;
};

#endif //MESH_H