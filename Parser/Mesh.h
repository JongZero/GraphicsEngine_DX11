#pragma once
#include "AlignedAllocationPolicy.h"
#include "ParserDefine.h"

/// <summary>
/// �޽� �ϳ��� �����͵�
/// 
/// �������� ASE Parser�� �Ű��
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

	// �ִϸ��̼� ����
	bool IsAnimated = false;				// �ִϸ��̼��� �ִٰ� �ϸ� ��� ������ �˻��ؼ� �ִϸ��̼� �����Ϳ� ��Ī��

	// ��Ű�� ����
	bool IsBone = false;
	bool IsSkinnedMesh = false;
	UINT BoneIndex = 0;
};
