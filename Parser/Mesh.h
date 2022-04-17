#pragma once
#include "AlignedAllocationPolicy.h"
#include "ParserDefine.h"

/// <summary>
/// 메쉬 하나의 데이터들
/// 
/// 정종영의 ASE Parser를 옮겼다
/// 2021. 11. 03 Hakgeum
/// </summary>
struct Mesh : public AlignedAllocationPolicy<16>
{
	int Type = 1;

	int IndexCount = 0;
	int StartIndex = 0;
	int StartVertex = 0;

	std::string NodeName;
	std::string NodeParentName;

	SimpleMath::Matrix LocalTM = SimpleMath::Matrix::Identity;
	SimpleMath::Matrix WorldTM = SimpleMath::Matrix::Identity;

	SimpleMath::Matrix NodeTM = SimpleMath::Matrix::Identity;
	SimpleMath::Matrix NodeInverseTM = SimpleMath::Matrix::Identity;

	Mesh* Parent = nullptr;
	UINT ParentCount = 0;

	// 애니메이션 정보
	bool IsAnimated = false;				// 애니메이션이 있다고 하면 노드 네임을 검사해서 애니메이션 데이터와 매칭함

	// 스키닝 정보
	bool IsBone = false;
	bool IsSkinnedMesh = false;
	UINT BoneIndex = 0;
};
