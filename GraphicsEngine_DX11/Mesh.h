#ifndef MESH_H
#define MESH_H

#include <string>
#include <windows.h>
#include "EMath.h"

/// <summary>
/// �޽� �ϳ��� �����͵�
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

	// �ִϸ��̼� ����
	bool IsAnimated = false;				// �ִϸ��̼��� �ִٰ� �ϸ� ��� ������ �˻��ؼ� �ִϸ��̼� �����Ϳ� ��Ī��
	EMath::Matrix AnimationTM = EMath::Matrix::Identity;	// ���� �������� �ִϸ��̼� TM

	// ��Ű�� ����
	bool IsBone = false;
	bool IsSkinnedMesh = false;
	UINT BoneIndex = 0;
};

#endif //MESH_H