#pragma once
#include <string>
#include <vector>
#include "EMath.h"

namespace ParserData
{
	struct Vertex
	{
		// �⺻
		EMath::Vector3 m_Pos;
		EMath::Vector3 m_Normal;
		EMath::Vector2 m_UV;
		int m_Index = 0;						// ���ؽ� �迭���� �ڽ��� �ε���

		// �븻��
		EMath::Vector3 m_Tangent;

		/// Skinned Mesh ���� �߰�
		// ��Ű��
		int m_BoneSize = 0;
		EMath::Vector3 m_BoneWeight;
		EMath::Vector4 m_BoneIndex;

		bool operator< (const Vertex& rhs) const
		{
			// Normal�� �ٸ� ���
			if (m_Normal != rhs.m_Normal)
			{
				if (m_Normal.x != rhs.m_Normal.x)
				{
					return m_Normal.x > rhs.m_Normal.x;
				}
				
				if (m_Normal.y != rhs.m_Normal.y)
				{
					return m_Normal.y > rhs.m_Normal.y;
				}

				return m_Normal.z > rhs.m_Normal.z;
			}

			// Pos�� �ٸ� ���
			if (m_Pos != rhs.m_Pos)
			{
				if (m_Pos.x != rhs.m_Pos.x)
				{
					return m_Pos.x > rhs.m_Pos.x;
				}

				if (m_Pos.y != rhs.m_Pos.y)
				{
					return m_Pos.y > rhs.m_Pos.y;
				}

				return m_Pos.z > rhs.m_Pos.z;
			}

			// UV�� �ٸ� ���
			if (m_UV != rhs.m_UV)
			{
				if (m_UV.x != rhs.m_UV.x)
				{
					return m_UV.x > rhs.m_UV.x;
				}

				return m_UV.y > rhs.m_UV.y;
			}

			// ���� Vertex���
			return false;
		}
	};

	struct Mesh
	{
		/// Split ��
		std::vector<ParserData::Vertex> m_VertexVec;

		/// Split ��
		std::vector<ParserData::Vertex> m_OptVertexVec;
		std::vector<unsigned int> m_OptIndexVec;
		
		std::string m_NodeName;
		std::string m_NodeParentName;

		EMath::Matrix m_NodeTM;
		EMath::Matrix m_NodeInverseTM;
		EMath::Matrix m_LocalTM;

		unsigned int m_FaceCount = 0;

		// Negative Scale �˻��
		bool m_IsNegativeScale = false;

		// Bone
		bool m_IsBone = false;
		unsigned int m_BoneIndex = 0;

		// Skinned Mesh
		bool m_IsSkinnedMesh = false;
	};
}