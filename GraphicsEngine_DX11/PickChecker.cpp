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
	//�þ� ����(view space)���� ���� ������(picking ray)�� ����Ѵ�.
	//	p.r[0].128_f32[0]->P(0, 0)
	//	p.r[1].128_f32[1]->P(1, 1)
	float vx = (2.0f * x / ClientSize::GetWidth() - 1.0f) / dataForRender->m_Proj._11;
	float vy = (-2.0f * y / ClientSize::GetHeight() + 1.0f) / dataForRender->m_Proj._22;

	// �þ� ����(view space)�� ������(ray) ����
	EMath::Vector4 rayOrigin(0.0f, 0.0f, 0.0f, 1.0f);

	// �ָ� �ִ� ��ü���� �˻�� �ϴµ�, z�� ���� ����� ��ü�� pick�ǵ��� �Ѵ�.
	EMath::Vector4 rayDir(vx, vy, 1.0f, 0.0f);

	// ������(ray)�� �޽��� ���� ����(local space)�� ��ȯ�Ѵ�.
	EMath::Matrix invView = dataForRender->m_View.Invert();

	wcscpy_s(dataForRender->m_PickedObject, L"");
	float dist = 0;		// PickBox�� Ray�� �Ÿ�
	float minDist = FLT_MAX;	// �ּ� �Ÿ�
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

	// World x View x Proj���� View�� Proj�� ������� ���ϸ� World�� Ƣ��´�.
	EMath::Matrix toLocal = invView * invWorld;

	// rayOrigin �� rayDir�� �� ������Ʈ�� Local Space�� ������.
	// XMVector3TransformCoord : w = 1�̶�� �����ϰ� ����� ����
	EMath::Vector4 rayOriginLocal = EMath::Vector3::TransformCoord(rayOrigin, toLocal);
	// XMVector3TransformNormal : w = 0�̶�� �����ϰ� ����� ����
	EMath::Vector4 rayDirLocal = EMath::Vector3::TransformNormal(rayDir, toLocal);

	// ���� ������ ���� ������ ����(ray Dir) ���͸� ���� ���̷� �����.
	rayDirLocal.Normalize();

	ECollision::Ray _ray(rayOriginLocal, rayDirLocal);

	// ������(ray)�� �޽��� ������(collider box)�� �����ϴ� ��쿡�� �޽��� �ﰢ���鿡 ���� ������ �� �ﰢ�� ���� ������ �����Ѵ�.
	// ��� ����(collider box)�� �������� �ʴ´ٸ� ������(ray)�� �޽ÿ� ������ ���ɼ��� �����Ƿ�,
	// ������(ray) �� �ﰢ�� ���� �������� �ð��� ������ ������ ����.
	float dist = FLT_MAX;

	if (objMeshData->m_BoundingBoxForPicking.Intersects(rayOriginLocal, rayDirLocal, dist))
	{
		for (UINT i = 0; i < objMeshData->m_IndexCount / 3; i++)
		{
			// �� �ﰢ��(face)�� �ε�����
			UINT index0 = objMeshData->m_IndexVec[i * 3 + 0];
			UINT index1 = objMeshData->m_IndexVec[i * 3 + 1];
			UINT index2 = objMeshData->m_IndexVec[i * 3 + 2];

			// �� �ﰢ���� vertex��
			EMath::Vector3 v0 = objMeshData->m_VertexPosVec[index0];
			EMath::Vector3 v1 = objMeshData->m_VertexPosVec[index1];
			EMath::Vector3 v2 = objMeshData->m_VertexPosVec[index2];

			// �޽��� ��� �ﰢ���� �����Ѵ�.
			if (_ray.Intersects(v0, v1, v2, dist))
			{
				return dist;
			}
		}
	}

	return 0;
}
