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
	// 메쉬데이터를 가져옴
	ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(objName);

	// 메쉬가 없는데 애니메이션 시킬 수 없다.
	if (_objMeshData != nullptr)
	{
		// 모션 데이터를 가져옴
		std::unordered_map<std::wstring, Motion*> _motionDataUMap;
		_motionDataUMap = m_ResourceManager->GetObjectMotionData(objName);

		// 모션 데이터가 있다면
		if (_motionDataUMap.size() > 0)
		{
			SetParent(_motionDataUMap, _objMeshData);
			InitializeAnimation(_motionDataUMap, _objMeshData);

			// 스키닝 오브젝트라면
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
		/// 메쉬들의 부모를 연결해준다.
		for (UINT i = 0; i < objMeshData->m_MeshVec.size(); i++)
		{
			// 부모가 없으면 넘어감
			if (0 == objMeshData->m_MeshVec[i]->NodeParentName.compare(""))
				continue;

			for (UINT j = 0; j < objMeshData->m_MeshVec.size(); j++)
			{
				// 이름을 기반으로 부모를 찾아 연결해준다.
				if (objMeshData->m_MeshVec[i]->NodeParentName == objMeshData->m_MeshVec[j]->NodeName)
				{
					objMeshData->m_MeshVec[i]->Parent = objMeshData->m_MeshVec[j];
					break;
				}
			}
		}

		/// 부모가 몇 개인지 체크한다.
		for (UINT i = 0; i < objMeshData->m_MeshVec.size(); i++)
		{
			// 부모가 없으면 넘어감
			if (0 == objMeshData->m_MeshVec[i]->NodeParentName.compare(""))
				continue;

			// 부모가 있으면 타고 타고 올라가 부모가 몇 개인지 체크
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

		/// 부모가 최대 몇 개인지 체크
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
		it.second->m_LastKeyFrame = 0;			// 각각의 애니메이션 데이터에 들어있는 Animation TM의 사이즈, 이를 통해 마지막 프레임이 몇인지를 측정

		if (it.second->m_AnimationDataVec.size() > 0)
		{
			// 마지막 프레임이 몇인지를 체크한다.
			it.second->m_LastKeyFrame = it.second->m_AnimationDataVec.front()->m_AnimationTMVec.size() - 1;
		}

		for (const auto& j : it.second->m_AnimationDataVec)
		{
			for (const auto& k : objMeshData->m_MeshVec)
			{
				// Mesh와 Animation Data를 매칭시킨다.
				// (Mesh의 Parent 및 WorldTM, NodeTm... 에 접근하기 위해서
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
			// 본인지 체크하고 본이라면 본 벡터에 넣는다.
			if (objMeshData->m_MeshVec[i]->IsBone)
			{
				it.second->m_BoneVec.push_back(objMeshData->m_MeshVec[i]);
			}
			else
			{
				// 스키닝 메쉬 체크
				if (objMeshData->m_MeshVec[i]->IsSkinnedMesh)
				{
					it.second->m_SkinnedMesh = objMeshData->m_MeshVec[i];
				}
			}
		}

		// 본 인덱스 정렬하기 (인덱스 작은 순서부터)
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
	// 이 함수가 호출되었다면 이미 메쉬와 애니메이션 데이터가 존재하고 초기화된 것임

	// 메쉬데이터를 가져옴
	ObjectMeshData* _objMeshData = m_ResourceManager->GetObjectMeshData(objName);

	// 모션 데이터를 가져옴
	std::unordered_map<std::wstring, Motion*> _motionDataUMap = m_ResourceManager->GetObjectMotionData(objName);

	// 현재 모션 이름에 맞는 데이터가 있다면 가져옴
	if (_motionDataUMap.count(nowMotionName) > 0)
	{
		// 현재 모션을 가져옴
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

		// 애니메이션 데이터를 순회하면서 애니메이션을 시킨다.
		for (MeshAnimationData* animData : _nowMotion->m_AnimationDataVec)
		{
			if (animData->m_Mesh != nullptr)
			{
				// 현재 키프레임의 애니메이션 TM을 가져온다.
				if (animData->m_AnimationTMVec.size() > 0)
				{
					animData->m_Mesh->AnimationTM = animData->m_AnimationTMVec[_nowMotion->m_NowKeyFrame];
				}
			}
		}

		/// 부모가 적은 순서부터 (상위 계층부터) 차례대로 World TM을 곱해준다.
		for (UINT j = 0; j <= _nowMotion->m_MaxParentCount; j++)
		{
			for (UINT i = 0; i < _objMeshData->m_MeshVec.size(); i++)
			{
				if (_objMeshData->m_MeshVec[i]->ParentCount == j)
				{
					// 최상위 일 경우
					if (j == 0)
					{
						_objMeshData->m_MeshVec[i]->WorldTM = _objMeshData->m_MeshVec[i]->AnimationTM * _objMeshData->m_MeshVec[i]->LocalTM;
					}
					// 부모가 있을 경우
					else
					{
						_objMeshData->m_MeshVec[i]->WorldTM = _objMeshData->m_MeshVec[i]->AnimationTM * _objMeshData->m_MeshVec[i]->Parent->WorldTM;
					}
				}
			}
		}

		// 스키닝 오브젝트인 경우에
		if (_objMeshData->m_ObjectMeshInfo.m_EffectType == ObjectMeshInfo::eEffectType::Skinning)
		{
			// BoneTransform 벡터 비우기
			_objMeshData->m_BoneFinalTransformVec.clear();

			/// VS로 보낼 최종 Bone TM 구하기
			for (UINT i = 0; i < _nowMotion->m_BoneVec.size(); i++)
			{
				EMath::Matrix boneWorldTM = _nowMotion->m_BoneVec[i]->WorldTM;
				EMath::Matrix boneNodeTM = _nowMotion->m_BoneVec[i]->NodeTM;

				EMath::Matrix skinWorldTM = _nowMotion->m_SkinnedMesh->NodeTM;

				EMath::Matrix skinWorldInverseTM = skinWorldTM.Invert();

				// Bone Offset TM은 해당 Bone의 최초위치를 나타내는 TM이다.
				// Bone Offset TM = Bone Node * Skin World의 역행렬
				// Skinned Mesh 기준의 Bone의 LocalTM
				EMath::Matrix boneOffsetTM = boneNodeTM * skinWorldInverseTM;
				EMath::Matrix boneOffsetInverseTM = boneOffsetTM.Invert();

				// Bone Offset TM의 역행렬 * Bone TM
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
