#include "PickChecker.h"
#include "ClientSize.h"
#include "ObjectMeshData.h"
#include "ResourceManager.h"
#include "ECollision.h"
#include "Mesh.h"
#include "MathConverter.h"
#include "Vertex.h"


#include "MathConverter.h"

PickChecker::PickChecker(std::shared_ptr<ResourceManager> rm)
	: m_ResourceManager(rm)
{

}

PickChecker::~PickChecker()
{

}

void PickChecker::PickCheck(int x, int y, Shared_RenderingData* dataForRender)
{
	//시야 공간(view space)에서 선택 반직선(picking ray)을 계산한다.
	//	p.r[0].128_f32[0]->P(0, 0)
	//	p.r[1].128_f32[1]->P(1, 1)
	float vx = (2.0f * x / ClientSize::GetWidth() - 1.0f) / dataForRender->m_Proj._11;
	float vy = (-2.0f * y / ClientSize::GetHeight() + 1.0f) / dataForRender->m_Proj._22;

	// 시야 공간(view space)의 반직선(ray) 정의
	EMath::Vector4 rayOrigin(0.0f, 0.0f, 0.0f, 1.0f);

	// 멀리 있는 물체까지 검사는 하는데, z가 가장 가까운 물체가 pick되도록 한다.
	EMath::Vector4 rayDir(vx, vy, 1.0f, 0.0f);

	// 반직선(ray)을 메시의 국소 공간(local space)로 변환한다.
	EMath::Matrix invView = dataForRender->m_View.Invert();

	wcscpy_s(dataForRender->m_PickedObject, L"");
	float dist = 0;		// PickBox와 Ray의 거리
	float minDist = FLT_MAX;	// 최소 거리
	for (auto it : dataForRender->m_ObjectDataForRenderVec)
	{
		ObjectMeshData* objMeshData = m_ResourceManager->GetObjectMeshData(it->m_Name);
		if (objMeshData == nullptr)
			continue;

		dist = PickCheck(it, rayOrigin, rayDir, invView);

		if (dist > 0 && dist < minDist)
		{
			minDist = dist;
			wcscpy_s(dataForRender->m_PickedObject, it->m_Name);
			
		}
	}
}

float PickChecker::PickCheck(Shared_ObjectData* objDataForRender, const EMath::Vector4& rayOrigin, const EMath::Vector4& rayDir, const EMath::Matrix& invView)
{
	ObjectMeshData* objMeshData = m_ResourceManager->GetObjectMeshData(objDataForRender->m_Name);

	EMath::Matrix W = objDataForRender->m_World;
	//W = W * objMeshData->m_MeshVec[0]->NodeTM;

	EMath::Matrix invWorld = W.Invert();

	// World x View x Proj에서 View와 Proj의 역행렬을 곱하면 World가 튀어나온다.
	EMath::Matrix toLocal = invView * invWorld;

	// rayOrigin 과 rayDir을 그 오브젝트의 Local Space로 보낸다.
	// XMVector3TransformCoord : w = 1이라고 가정하고 계산을 수행
	EMath::Vector4 rayOriginLocal = EMath::Vector3::TransformCoord(rayOrigin, toLocal);
	// XMVector3TransformNormal : w = 0이라고 가정하고 계산을 수행
	EMath::Vector4 rayDirLocal = EMath::Vector3::TransformNormal(rayDir, toLocal);

	// 교차 판정을 위해 반직선 방향(ray Dir) 벡터를 단위 길이로 만든다.
	rayDirLocal.Normalize();

	ECollision::Ray _ray(rayOriginLocal, rayDirLocal);

	// 반직선(ray)이 메시의 경계상자(collider box)와 교차하는 경우에만 메시의 삼각형들에 대해 반직선 대 삼각형 교차 판정을 수행한다.
	// 경계 상자(collider box)와 교차하지 않는다면 반직선(ray)이 메시와 교차할 가능성이 없으므로,
	// 반직선(ray) 대 삼각형 교차 판정으로 시간을 낭비할 이유가 없다.
	float dist = FLT_MAX;

	if (objMeshData->m_BoundingBoxForPicking.Intersects(rayOriginLocal, rayDirLocal, dist))
	{
		for (UINT i = 0; i < objMeshData->m_IndexCount / 3; i++)
		{
			// 이 삼각형(face)의 인덱스들
			UINT index0 = objMeshData->m_IndexVec[i * 3 + 0];
			UINT index1 = objMeshData->m_IndexVec[i * 3 + 1];
			UINT index2 = objMeshData->m_IndexVec[i * 3 + 2];

			// 이 삼각형의 vertex들
			EMath::Vector3 v0 = objMeshData->m_VertexPosVec[index0];
			EMath::Vector3 v1 = objMeshData->m_VertexPosVec[index1];
			EMath::Vector3 v2 = objMeshData->m_VertexPosVec[index2];

			// 메시의 모든 삼각형을 판정한다.
			if (_ray.Intersects(v0, v1, v2, dist))
			{
				return dist;
			}
		}
	}

	return 0;
}
