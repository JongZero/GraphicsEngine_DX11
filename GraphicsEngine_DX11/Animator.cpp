#include "Animator.h"
#include "ResourceManager.h"
#include "Mesh.h"
#include "ObjectMeshData.h"
#include "MeshAnimationData.h"

#include <algorithm>

Animator::Animator(std::shared_ptr<ResourceManager> rm)
	: m_ResourceManager(rm)
{

}

Animator::~Animator()
{

}

void Animator::InitializeAnimation(const std::wstring& objName, bool& isCompleted)
{
	// �޽������͸� ������
	ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(objName);

	// �޽��� ���µ� �ִϸ��̼� ��ų �� ����.
	if (_objMeshData != nullptr)
	{
		// ��� �����͸� ������
		std::unordered_map<std::wstring, Motion*> _motionDataUMap;
		_motionDataUMap = m_ResourceManager->GetObjectMotionData(objName);

		// ��� �����Ͱ� �ִٸ�
		if (_motionDataUMap.size() > 0)
		{
			SetParent(_motionDataUMap, _objMeshData);
			InitializeAnimation(_motionDataUMap, _objMeshData);

			// ��Ű�� ������Ʈ���
			if (_objMeshData->m_ObjectMeshInfo.m_EffectType == ObjectMeshInfo::eEffectType::Skinning)
			{
				SetBoneAndSkinnedMesh(_motionDataUMap, _objMeshData);
			}

			isCompleted = true;
			return;
		}
	}

	isCompleted = false;
}

void Animator::SetParent(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData)
{
	for (const auto& it : motionDataUMap)
	{
		/// �޽����� �θ� �������ش�.
		for (UINT i = 0; i < objMeshData->m_MeshVec.size(); i++)
		{
			// �θ� ������ �Ѿ
			if (0 == objMeshData->m_MeshVec[i]->NodeParentName.compare(""))
				continue;

			for (UINT j = 0; j < objMeshData->m_MeshVec.size(); j++)
			{
				// �̸��� ������� �θ� ã�� �������ش�.
				if (objMeshData->m_MeshVec[i]->NodeParentName == objMeshData->m_MeshVec[j]->NodeName)
				{
					objMeshData->m_MeshVec[i]->Parent = objMeshData->m_MeshVec[j];
					break;
				}
			}
		}

		/// �θ� �� ������ üũ�Ѵ�.
		for (UINT i = 0; i < objMeshData->m_MeshVec.size(); i++)
		{
			// �θ� ������ �Ѿ
			if (0 == objMeshData->m_MeshVec[i]->NodeParentName.compare(""))
				continue;

			// �θ� ������ Ÿ�� Ÿ�� �ö� �θ� �� ������ üũ
			Mesh* mesh = objMeshData->m_MeshVec[i];
			while (true)
			{
				if (IsParentExist(mesh))
				{
					objMeshData->m_MeshVec[i]->ParentCount++;
					mesh = mesh->Parent;
				}
				else
					break;
			}
		}

		/// �θ� �ִ� �� ������ üũ
		for (UINT i = 0; i < objMeshData->m_MeshVec.size(); i++)
		{
			if (objMeshData->m_MeshVec[i]->ParentCount > it.second->m_MaxParentCount)
				it.second->m_MaxParentCount = objMeshData->m_MeshVec[i]->ParentCount;
		}
	}
}

bool Animator::IsParentExist(Mesh* mesh)
{
	if (mesh->Parent == nullptr)
		return false;
	else
		return true;
}

void Animator::InitializeAnimation(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData)
{
	for (const auto& it : motionDataUMap)
	{
		it.second->m_LastKeyFrame = 0;			// ������ �ִϸ��̼� �����Ϳ� ����ִ� Animation TM�� ������, �̸� ���� ������ �������� �������� ����

		if (it.second->m_AnimationDataVec.size() > 0)
		{
			// ������ �������� �������� üũ�Ѵ�.
			it.second->m_LastKeyFrame = it.second->m_AnimationDataVec.front()->m_AnimationTMVec.size() - 1;
		}

		for (const auto& j : it.second->m_AnimationDataVec)
		{
			for (const auto& k : objMeshData->m_MeshVec)
			{
				// Mesh�� Animation Data�� ��Ī��Ų��.
				// (Mesh�� Parent �� WorldTM, NodeTm... �� �����ϱ� ���ؼ�
				if (k->NodeName == j->m_NodeName)
				{
					j->m_Mesh = k;
					j->m_ParentMesh = k->Parent;

					j->m_Mesh->IsAnimated = true;
					break;
				}
			}
		}
	}
}

void Animator::SetBoneAndSkinnedMesh(std::unordered_map<std::wstring, Motion*>& motionDataUMap, ObjectMeshData* objMeshData)
{
	for (const auto& it : motionDataUMap)
	{
		for (UINT i = 0; i < objMeshData->m_MeshVec.size(); i++)
		{
			// ������ üũ�ϰ� ���̶�� �� ���Ϳ� �ִ´�.
			if (objMeshData->m_MeshVec[i]->IsBone)
			{
				it.second->m_BoneVec.push_back(objMeshData->m_MeshVec[i]);
			}
			else
			{
				// ��Ű�� �޽� üũ
				if (objMeshData->m_MeshVec[i]->IsSkinnedMesh)
				{
					it.second->m_SkinnedMesh = objMeshData->m_MeshVec[i];
				}
			}
		}

		// �� �ε��� �����ϱ� (�ε��� ���� ��������)
		sort(it.second->m_BoneVec.begin(), it.second->m_BoneVec.end(),
			[](Mesh* bone1, Mesh* bone2) -> bool
			{
				return bone1->BoneIndex < bone2->BoneIndex;
			});
	}
}

void Animator::UpdateAnimation(const std::wstring& objName, const std::wstring& nowMotionName, 
	float dTime, float speed)
{
	// �� �Լ��� ȣ��Ǿ��ٸ� �̹� �޽��� �ִϸ��̼� �����Ͱ� �����ϰ� �ʱ�ȭ�� ����

	// �޽������͸� ������
	ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(objName);

	// ��� �����͸� ������
	std::unordered_map<std::wstring, Motion*> _motionDataUMap = m_ResourceManager->GetObjectMotionData(objName);

	// ���� ��� �̸��� �´� �����Ͱ� �ִٸ� ������
	if (_motionDataUMap.count(nowMotionName) > 0)
	{
		// ���� ����� ������
		Motion* _nowMotion = _motionDataUMap[nowMotionName];

		_nowMotion->m_NowTime += dTime * speed;
		//_nowMotion->m_NowKeyFrame = (int)_nowMotion->m_NowTime;

		if (_nowMotion->m_NowTime > _nowMotion->m_TicksPerFrame)
		{
			_nowMotion->m_NowKeyFrame++;
			_nowMotion->m_NowTime = 0.0f;
		}

		if (_nowMotion->m_NowKeyFrame > _nowMotion->m_LastKeyFrame)
		{
			_nowMotion->m_NowKeyFrame = _nowMotion->m_FirstKeyFrame;
		}

		// �ִϸ��̼� �����͸� ��ȸ�ϸ鼭 �ִϸ��̼��� ��Ų��.
		for (MeshAnimationData* animData : _nowMotion->m_AnimationDataVec)
		{
			if (animData->m_Mesh != nullptr)
			{
				// ���� Ű�������� �ִϸ��̼� TM�� �����´�.
				if (animData->m_AnimationTMVec.size() > 0)
				{
					animData->m_Mesh->AnimationTM = animData->m_AnimationTMVec[_nowMotion->m_NowKeyFrame];
				}
			}
		}

		/// �θ� ���� �������� (���� ��������) ���ʴ�� World TM�� �����ش�.
		for (UINT j = 0; j <= _nowMotion->m_MaxParentCount; j++)
		{
			for (UINT i = 0; i < _objMeshData->m_MeshVec.size(); i++)
			{
				if (_objMeshData->m_MeshVec[i]->ParentCount == j)
				{
					// �ֻ��� �� ���
					if (j == 0)
					{
						_objMeshData->m_MeshVec[i]->WorldTM = _objMeshData->m_MeshVec[i]->AnimationTM * _objMeshData->m_MeshVec[i]->LocalTM;
					}
					// �θ� ���� ���
					else
					{
						_objMeshData->m_MeshVec[i]->WorldTM = _objMeshData->m_MeshVec[i]->AnimationTM * _objMeshData->m_MeshVec[i]->Parent->WorldTM;
					}
				}
			}
		}

		// ��Ű�� ������Ʈ�� ��쿡
		if (_objMeshData->m_ObjectMeshInfo.m_EffectType == ObjectMeshInfo::eEffectType::Skinning)
		{
			// BoneTransform ���� ����
			_objMeshData->m_BoneFinalTransformVec.clear();

			/// VS�� ���� ���� Bone TM ���ϱ�
			for (UINT i = 0; i < _nowMotion->m_BoneVec.size(); i++)
			{
				EMath::Matrix boneWorldTM = _nowMotion->m_BoneVec[i]->WorldTM;
				EMath::Matrix boneNodeTM = _nowMotion->m_BoneVec[i]->NodeTM;

				EMath::Matrix skinWorldTM = _nowMotion->m_SkinnedMesh->NodeTM;

				EMath::Matrix skinWorldInverseTM = skinWorldTM.Invert();

				// Bone Offset TM�� �ش� Bone�� ������ġ�� ��Ÿ���� TM�̴�.
				// Bone Offset TM = Bone Node * Skin World�� �����
				// Skinned Mesh ������ Bone�� LocalTM
				EMath::Matrix boneOffsetTM = boneNodeTM * skinWorldInverseTM;
				EMath::Matrix boneOffsetInverseTM = boneOffsetTM.Invert();

				// Bone Offset TM�� ����� * Bone TM
				EMath::Matrix finalBoneTM = boneOffsetInverseTM * boneWorldTM;
				_objMeshData->m_BoneFinalTransformVec.push_back(finalBoneTM);
			}
		}
	}
}

void Animator::ResetMotion(Motion* motion)
{
	motion->m_NowKeyFrame = motion->m_FirstKeyFrame;
	motion->m_NowTime = motion->m_NowKeyFrame;

	for (auto& j : motion->m_AnimationDataVec)
	{
		j->m_PosKeyFrameIndex = 0;
		j->m_RotKeyFrameIndex = 0;

		if (j->m_PosSampleVec.size() > 0)
		{
			j->m_StartPosSample = j->m_PosSampleVec[j->m_PosKeyFrameIndex];
			j->m_NowPosSample = j->m_PosSampleVec[j->m_PosKeyFrameIndex];
			j->m_NextPosSample = j->m_PosSampleVec[j->m_PosKeyFrameIndex + 1];

			j->m_PosStartKeyFrame = j->m_PosKeyFrameVec[j->m_PosKeyFrameIndex];
			j->m_PosNowKeyFrame = j->m_PosStartKeyFrame;
			j->m_PosNextKeyFrame = j->m_PosKeyFrameVec[j->m_PosKeyFrameIndex + 1];
		}

		if (j->m_RotSampleVec.size() > 0)
		{
			j->m_StartRotSample = j->m_RotSampleVec[j->m_RotKeyFrameIndex];
			j->m_NowRotSample = j->m_RotSampleVec[j->m_RotKeyFrameIndex];
			j->m_NextRotSample = j->m_RotSampleVec[j->m_RotKeyFrameIndex + 1];

			j->m_RotStartKeyFrame = j->m_RotKeyFrameVec[j->m_RotKeyFrameIndex];
			j->m_RotNowKeyFrame = j->m_RotStartKeyFrame;
			j->m_RotNextKeyFrame = j->m_RotKeyFrameVec[j->m_RotKeyFrameIndex + 1];
		}

		j->m_Mesh->IsAnimated = true;
	}
}
