#include "ASEParser.h"
#include "Mathhelper.h"
#include <fstream>

ASEParser::ASEParser()
	:m_parsingmode(eNone), m_MaterialCount(0), m_MaterialVec(), m_MeshVec(),
	m_NowMaterial(nullptr), m_NowMesh(nullptr), m_NowTokenID(), m_TokenString(),
	m_data_asciiexport(0), m_nowshapeline(nullptr), m_nowshapevertex(nullptr),
	m_IsFirstNodeName(false), m_NowMotionData(nullptr), m_FaceIndex(0), m_FaceVertexIndex(0)
{
	m_Lexer = new CLexer("", 0);
}

ASEParser::~ASEParser()
{

}

bool ASEParser::Load(std::string fileName)
{
	std::string path = "../../Data/ASE/";

	/// 0) ������ �ε��Ѵ�.
	if (!m_Lexer->Open(path + fileName))
	{
		TRACE("������ ���� �߿� ������ �߻��߽��ϴ�!");
		return FALSE;
	}

	m_MeshVecMap[fileName] = new std::vector<ASEMeshData*>;
	m_MeshVec = m_MeshVecMap[fileName];

	m_MaterialVecMap[fileName] = new std::vector<ASEMaterial*>;
	m_MaterialVec = m_MaterialVecMap[fileName];

	m_NowMotionData = new Motion;

	// ����ͷ� ASE������ �Ľ��Ѵ�.
	ParseASE();

	for (UINT i = 0; i < m_MeshVec->size(); i++)
	{
		//ConvertAll(m_MeshVec->at(i));
		OptimiseMesh(m_MeshVec->at(i));
		ConvertVertexToLocalCoordinate(m_MeshVec->at(i));
	}

	m_BoneNameVec.clear();

	m_IsFirstNodeName = false;

	/// ��� ������
	// �ִϸ��̼� �����Ͱ� ���ٸ� �� �κ��� �Ѿ��.
	if (m_NowMotionData->m_AnimationDataVec.size() <= 0)
	{
		return TRUE;
	}

	// ù��° ����� �̸� �����س��� (������Ʈ ���п�)
	m_NowMotionData->m_ObjectNodeName = m_FirstNodeName;

	std::string motionName;

	// ���� �̸��� ������ _�� ���°���� ã�´�.
	size_t findIndex = fileName.find_last_of("_");

	// _�� ���ٸ� Default �̸����� ����
	if (findIndex == -1)
	{
		motionName = "None";
	}
	else
	{
		findIndex++;

		// ��� �̸��� Ȯ���ڸ� ����
		motionName = fileName.substr(findIndex, fileName.size() - findIndex);

		// Ȯ���� ����
		motionName = motionName.substr(0, motionName.size() - 4);
	}

	// ����� �̸��� ����
	m_NowMotionData->m_MotionName = motionName;

	/// ���� ������Ʈ���� üũ�ؾ���
	std::map<std::string, Motion*> motionMap = m_ObjectAnimationMap[m_FirstNodeName];

	// ���� �̸��� ������Ʈ�� �ʿ� ���ٸ�
	if (motionMap.size() <= 0)
	{
		motionMap[motionName] = m_NowMotionData;
		m_ObjectAnimationMap[m_FirstNodeName] = motionMap;
	}
	// �̹� �ִٸ� ���� ���Ϳ� �ִ´�. (���� ������Ʈ�̹Ƿ�)
	else
	{
		m_ObjectAnimationMap[m_FirstNodeName][motionName] = m_NowMotionData;
	}

	return TRUE;
}

bool ASEParser::ConvertAll(ASEMeshData* pMesh)
{
	// ���ؽ����� �ϴ� ��� ����
	for (unsigned int i = 0; i < pMesh->m_meshvertex.size(); i++)
	{
		AVertex* _pVertex = new AVertex;
		_pVertex->m_pos = pMesh->m_meshvertex[i]->m_pos;
		//_pVertex->m_normal = pMesh->m_meshvertex[i]->m_normal;
		_pVertex->u = pMesh->m_meshvertex[i]->u;
		_pVertex->v = pMesh->m_meshvertex[i]->v;
		_pVertex->m_bone_blending_weight = pMesh->m_meshvertex[i]->m_bone_blending_weight;

		pMesh->m_opt_vertex.push_back(_pVertex);
	}

	// �������� face�� �������� �� w�ε����� ã�ƾ� ��
	size_t _faceCount = pMesh->m_meshface.size();
	TRACE("FaceCount : %d", _faceCount);

	for (unsigned int i = 0; i < pMesh->m_meshface.size(); i++)
	{
		TRACE("FaceNum : %d / %d\n", i, _faceCount);

		for (int j = 0; j < 3; j++)
		{
			AVertex* _nowVertex = pMesh->m_opt_vertex[pMesh->m_meshface[i]->m_vertexindex[j]];

			// veretex�������� ���ؽ��� �븻�� �־��ش�.
			_nowVertex->m_normal.x = pMesh->m_meshface[i]->m_VertexNormal[j].x;
			_nowVertex->m_normal.y = pMesh->m_meshface[i]->m_VertexNormal[j].y;
			_nowVertex->m_normal.z = pMesh->m_meshface[i]->m_VertexNormal[j].z;
		}
	}

	// �ε����� �׳� ����
	for (unsigned int i = 0; i < pMesh->m_meshface.size(); i++)
	{
		IndexList* indexList = new IndexList;

		for (int j = 0; j < 3; j++)
		{
			indexList->index[j] = pMesh->m_meshface[i]->m_vertexindex[j];
		}

		pMesh->m_opt_index.push_back(indexList);
	}

	return FALSE;
}

bool ASEParser::OptimiseMesh(ASEMeshData* mesh)
{
	static bool isVertexCreateEnd = false;		// for�� Ż��� ����

	// 6�� for���� ���鼭 vertex ���� �Է�, vertex ����, face index ������ ��� �� ���� �Ѵ�.
	// 2�� for�� x 3 �̶�� ���� �ȴ�.
	// ù��° 2�� for�� : face vector�� ��ȸ�ϸ鼭 face�� ������ִ� vertex������ �ְ� index�� �����Ѵ�.
	// �ι�° 2�� for�� : ���� face index(i) ���� ���� face���� ��ȸ�ϸ鼭 ������ vertex�� �ִ��� �˻��Ѵ�.
	// ����° 2�� for�� : ���� face index(i) ���� ū face���� ��ȸ�ϸ鼭 ������ vertex�� ���ο� index�� �־��ش�.
	for (UINT i = 0; i < mesh->m_meshface.size(); i++)
	{
		IndexList* indexList = new IndexList;

		for (UINT j = 0; j < 3; j++)
		{
			// normal ��
			int checkIndex = mesh->m_meshface[i]->m_vertexindex[j];			// ���� face�� j��° �ε���
			SimpleMath::Vector3 checkNormal = mesh->m_meshface[i]->m_VertexNormal[j];	// ���� face�� j��° �븻

			// u,v ��
			float checkU = -1;		// ���� face�� j��° tVertex �ε����� u ��
			float checkV = -1;		// ���� face�� j��° tVertex �ε����� v ��

			// tvertex ������ ���� ase ������ �Ľ��� �� ����ó��
			if (mesh->m_mesh_tvertex.size() > 0)
			{
				checkU = mesh->m_mesh_tvertex[mesh->m_meshface[i]->m_TFace[j]]->m_u;
				checkV = mesh->m_mesh_tvertex[mesh->m_meshface[i]->m_TFace[j]]->m_v;
			}

			// ������ Face��� ���Ѵ�.
			for (UINT k = 0; k < i; k++)
			{
				// vertex�� index, normal, uv 3�� ��� ��
				for (UINT l = 0; l < 3; l++)
				{
					// �ε����� ����
					if (checkIndex == mesh->m_meshface[k]->m_vertexindex[l])
					{
						// �븻�� �ٸ���
						if (checkNormal != mesh->m_meshface[k]->m_VertexNormal[l])
						{
							// ���ο� ���ؽ� ����
							AVertex* newVertex = new AVertex;

							// pos, uv�� ����, weight�� ����
							newVertex->m_pos = mesh->m_meshvertex[checkIndex]->m_pos;
							newVertex->u = mesh->m_meshvertex[checkIndex]->u;
							newVertex->v = mesh->m_meshvertex[checkIndex]->v;
							newVertex->m_bone_blending_weight = mesh->m_meshvertex[checkIndex]->m_bone_blending_weight;

							// normal�� �ٸ���
							newVertex->m_normal = checkNormal;

							// �ε����� ���� ����
							newVertex->m_indices = mesh->m_meshvertex.size();
							mesh->m_meshvertex.push_back(newVertex);

							// �ε��� �ٽ� �̾��ֱ�
							for (UINT q = i; q < mesh->m_meshface.size(); q++)
							{
								for (UINT w = j; w < 3; w++)
								{
									// �˻��� �ε���
									int checkIndexForChange = mesh->m_meshface[q]->m_vertexindex[w];

									// �˻��� �븻 ��
									SimpleMath::Vector3 checkNormalForChange = mesh->m_meshface[q]->m_VertexNormal[w];

									// �ε����� u,v�� ��� ������ ���ο� vertex�� �ε��� ������ �־��ش�.
									if (checkIndex == checkIndexForChange &&
										checkNormal == checkNormalForChange)
									{
										mesh->m_meshface[q]->m_vertexindex[w] = newVertex->m_indices;
									}
								}
							}

							// for�� Ż���
							isVertexCreateEnd = true;
							break;
						}

						// u,v�� �ٸ���
						if (mesh->m_mesh_tvertex.size() > 0 &&
							checkU != mesh->m_mesh_tvertex[mesh->m_meshface[k]->m_TFace[l]]->m_u &&
							checkV != mesh->m_mesh_tvertex[mesh->m_meshface[k]->m_TFace[l]]->m_v)
						{
							AVertex* newVertex = new AVertex;

							// pos, normal�� ����, weight�� ����
							newVertex->m_pos = mesh->m_meshvertex[checkIndex]->m_pos;
							newVertex->m_normal = mesh->m_meshvertex[checkIndex]->m_normal;
							newVertex->m_bone_blending_weight = mesh->m_meshvertex[checkIndex]->m_bone_blending_weight;

							// u, v�� �ٸ���
							newVertex->u = checkU;
							newVertex->v = checkV;

							// �ε����� ���� ����
							newVertex->m_indices = mesh->m_meshvertex.size();
							mesh->m_meshvertex.push_back(newVertex);

							// �ε��� �ٽ� �̾��ֱ� (���� face index�� �������͸� �˻�)
							for (UINT q = i; q < mesh->m_meshface.size(); q++)
							{
								for (UINT w = j; w < 3; w++)
								{
									// �˻��� �ε���
									int checkIndexForChange = mesh->m_meshface[q]->m_vertexindex[w];

									// �˻��� u, v��
									float checkUForChange = mesh->m_mesh_tvertex[mesh->m_meshface[q]->m_TFace[w]]->m_u;
									float checkVForChange = mesh->m_mesh_tvertex[mesh->m_meshface[q]->m_TFace[w]]->m_v;

									// �ε����� u,v�� ��� ������ ���ο� vertex�� �ε��� ������ �־��ش�.
									if (checkIndex == checkIndexForChange &&
										checkU == checkUForChange && checkV == checkVForChange)
									{
										mesh->m_meshface[q]->m_vertexindex[w] = newVertex->m_indices;
									}
								}
							}

							isVertexCreateEnd = true;
							break;
						}
					}
				}

				// for�� Ż��
				if (isVertexCreateEnd)
				{
					isVertexCreateEnd = false;
					break;
				}
			}

			//���� face�� j��° vertex
			AVertex* _nowVertex = mesh->m_meshvertex[mesh->m_meshface[i]->m_vertexindex[j]];

			//face�� ��ȸ�ϸ鼭 vertex�� �븻 ���� �־��ش�.
			_nowVertex->m_normal.x = mesh->m_meshface[i]->m_VertexNormal[j].x;
			_nowVertex->m_normal.y = mesh->m_meshface[i]->m_VertexNormal[j].y;
			_nowVertex->m_normal.z = mesh->m_meshface[i]->m_VertexNormal[j].z;

			// u, v ���� �־��ش�.
			if (mesh->m_mesh_tvertex.size() > 0)
			{
				_nowVertex->u = mesh->m_mesh_tvertex[mesh->m_meshface[i]->m_TFace[j]]->m_u;
				_nowVertex->v = mesh->m_mesh_tvertex[mesh->m_meshface[i]->m_TFace[j]]->m_v;
			}

			//�ε����� �־��ش�.
			indexList->index[j] = mesh->m_meshface[i]->m_vertexindex[j];
		}

		// 3���� �ε����� �� �־������� vector�� �ִ´�.
		mesh->m_opt_index.push_back(indexList);
	}

	// ���ؽ��� �״�� �����Ѵ�.
	mesh->m_opt_vertex.assign(mesh->m_meshvertex.begin(), mesh->m_meshvertex.end());

	return FALSE;
}

bool ASEParser::OptimiseMesh3(ASEMeshData* pMesh)
{
	// 2021.04.12
	// ���� ���� (���ÿ�)
	// �̰��� �޽ð� �ƴ϶�� ����

	unsigned int _faceCount = pMesh->m_meshface.size();
	pMesh->m_opt_index.resize(pMesh->m_meshface.size());

	for (unsigned int i = 0; i < pMesh->m_meshface.size(); i++)
	{
		pMesh->m_opt_index[i] = new IndexList;

		for (int j = 0; j < 3; j++)
		{
			// 1. Vertex�� �����Ѵ�
			// ��, �����Ͱ� ������ �������� �ʴ´�
			AVertex* pvertex;
			pvertex = new AVertex;

			if (pMesh->m_meshvertex.size() != 0)
			{
				pvertex->m_pos = pMesh->m_meshvertex[pMesh->m_meshface[i]->m_vertexindex[j]]->m_pos;

				if (pMesh->m_type == eObjectType_SkinnedMesh)
				{
					pvertex->m_bw1 = pMesh->m_meshvertex[pMesh->m_meshface[i]->m_vertexindex[j]]->m_bw1;
					pvertex->m_bw2 = pMesh->m_meshvertex[pMesh->m_meshface[i]->m_vertexindex[j]]->m_bw2;
					pvertex->m_bw3 = pMesh->m_meshvertex[pMesh->m_meshface[i]->m_vertexindex[j]]->m_bw3;
					pvertex->m_indices = pMesh->m_meshvertex[pMesh->m_meshface[i]->m_vertexindex[j]]->m_indices;
				}
			}

			pvertex->m_normal = pMesh->m_meshface[i]->m_VertexNormal[j];

			if (pMesh->m_mesh_tvertex.size() != 0)
			{
				pvertex->u = pMesh->m_mesh_tvertex[pMesh->m_meshface[i]->m_TFace[j]]->m_u;
				pvertex->v = pMesh->m_mesh_tvertex[pMesh->m_meshface[i]->m_TFace[j]]->m_v;
			}

			// 2. ���ݱ��� ���յ� Vertex�߿� ���� Vertex�� �ִ��� �˻��Ѵ�
			int index = 0;
			for (unsigned int vlistindex = 0; vlistindex < pMesh->m_opt_vertex.size(); vlistindex++, index++)
			{
				// ���� ���ؽ��� ������ ���� ���յȰ��� �����ϰ� ������ ��������
				if (pMesh->m_opt_vertex[index]->m_pos == (pvertex)->m_pos &&
					pMesh->m_opt_vertex[index]->m_normal == (pvertex)->m_normal &&
					pMesh->m_opt_vertex[index]->u == (pvertex)->u &&
					pMesh->m_opt_vertex[index]->v == (pvertex)->v)
				{
					/// ��� ����
					///TRACE("���� ���ؽ� ���� : %d\n", index);
					delete pvertex;
					pvertex = nullptr;
					break;
				}
			}

			// vertex�� �������� �ʾҴٸ� ���� vertex�� ���ٴ� ���̹Ƿ�
			// ����Ʈ�� �߰��� ��, �ε����� �־��ش� ��ȣ�� index
			if (pvertex != NULL)
			{
				pMesh->m_opt_vertex.push_back(pvertex);
				//m_opt_index[i][j] = index;
				pMesh->m_opt_index[i]->index[j] = index;
			}
			else // vertex�� �����Ǿ��� �� -> ���� Vertex�� �ִٴ� ��, �׿� �ش��ϴ� �ε����� index��
			{
				//m_opt_index[i][j] = index;
				pMesh->m_opt_index[i]->index[j] = index;
			}
		}
	}

	pMesh->m_mesh_numfaces = pMesh->m_meshface.size();

	return true;
}

bool ASEParser::ConvertVertexToLocalCoordinate(ASEMeshData* mesh)
{
	// Node TM �� Node TM�� ������� ���Ѵ�.
	XMMATRIX scale = XMMatrixScaling(mesh->m_tm_scale.x, mesh->m_tm_scale.y, mesh->m_tm_scale.z);
	SimpleMath::Vector3 scaleAxis = mesh->m_tm_scaleaxis;
	float scaleAng = mesh->m_tm_scaleaxisang;

	// Negative Scale���� Ȯ��
	SimpleMath::Vector3 row0 = mesh->m_tm_row0;
	SimpleMath::Vector3 row1 = mesh->m_tm_row1;
	SimpleMath::Vector3 row2 = mesh->m_tm_row2;

	// [ Negative Scale Ȯ�� ��� ]
	// row0, row1, row2�� ���� x,y,z���� ��Ÿ���Ƿ� �� �� �� ���� '����'�� ���� �ٸ� �� ��� ������ �ݴ����� Ȥ�� �������� Ȯ���ϸ�ȴ�.
	// -> ���� �� ���� �ٸ� �� ���� �����Ѵ�. (������ ��� ���� �ݴ�, ����� ��� ���� ���� ����)
	bool isNegativeScale = false;
	SimpleMath::Vector3 crossResult = row0.Cross(row1);
	float checkNegativeScaleResult = crossResult.Dot(row2);

	// ������ ���� �ݴ��̴�.
	if (checkNegativeScaleResult < 0)
	{
		isNegativeScale = true;
	}

	XMMATRIX scaleRotationInverseTM = XMMatrixIdentity();
	XMMATRIX scaleRotationTM = XMMatrixIdentity();

	mesh->m_ScaleTm = scale;

	float angle = mesh->m_tm_rotangle;
	SimpleMath::Vector3 rotateAxis = mesh->m_tm_rotaxis;

	XMMATRIX rotation;
	if (rotateAxis != SimpleMath::Vector3(0, 0, 0) && angle != 0)
		rotation = XMMatrixRotationAxis(rotateAxis, angle);
	else
		rotation = XMMatrixIdentity();

	XMMATRIX translation = XMMatrixTranslation(mesh->m_tm_pos.x, mesh->m_tm_pos.y, mesh->m_tm_pos.z);

	XMMATRIX world = XMMatrixIdentity();
	world = scale * rotation * translation;

	mesh->m_NodeTm = world;

	// NodeTm ����� ���ϱ�
	XMVECTOR det = XMMatrixDeterminant(translation);
	XMMATRIX translationInverse = XMMatrixInverse(&det, translation);

	det = XMMatrixDeterminant(rotation);
	XMMATRIX rotationInverse = XMMatrixInverse(&det, rotation);

	det = XMMatrixDeterminant(scale);
	XMMATRIX scaleInverse = XMMatrixInverse(&det, scale);

	XMMATRIX worldInverse = translationInverse * rotationInverse * scaleInverse;
	mesh->m_NodeInverseTm = worldInverse;

	XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
	XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);

	XMVECTOR vMin = XMLoadFloat3(&vMinf3);
	XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

	for (UINT i = 0; i < mesh->m_opt_vertex.size(); i++)
	{
		// Pos�� �ִ�, �ּ� ���� (Collider Box ������ ����)
		XMVECTOR P = XMLoadFloat3(&mesh->m_opt_vertex[i]->m_pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);

		// ���ؽ����� world -> local�� ������.
		mesh->m_opt_vertex[i]->m_pos = DirectX::XMVector3Transform(mesh->m_opt_vertex[i]->m_pos, worldInverse);

		// �븻 ���� �������ش�.
		//if (isNegativeScale)
		//	mesh->m_opt_vertex[i]->m_normal *= -1;
	}

	// Pos �ִ�, �ּ� ����
	mesh->m_min_vertex = vMin;
	mesh->m_max_vertex = vMax;

	return true;
}

void ASEParser::ParseASE()
{
	int i = 0;
	LONG nowtoken;
	int faceIndex = 0;
	int wVertexIndex = 0;

	/// �̰��� �ϸ� �� ���� ��ū�� �а�, �� ������ ������ �� �� �ִ�.
	while (nowtoken = m_Lexer->GetToken(m_TokenString), nowtoken != TOKEND_END)
	{
		// �ϴ� �� ���� ��ū�� �а�, �װ��� ��ȣ �ݱⰡ �ƴ϶��.
		// �Ѿ�� ��ū�� ���� ó�����ش�.

		switch (nowtoken)
		{
		case TOKENR_HELPER_CLASS:
			break;

			//--------------------
			// 3DSMAX_ASCIIEXPORT
			//--------------------

		case TOKENR_3DSMAX_ASCIIEXPORT:
			m_data_asciiexport = Parsing_NumberLong();
			break;

			//--------------------
			// COMMENT
			//--------------------

		case TOKENR_COMMENT:
			Parsing_String();	// �׳� m_TokenString�� �о������ ���� ��.
			//AfxMessageBox( m_TokenString, NULL, NULL);		/// �ӽ÷� �ڸ�Ʈ�� ����غ���
			break;

			//--------------------
			// SCENE
			//--------------------

		case TOKENR_SCENE:
			break;

		case TOKENR_SCENE_FILENAME:
			m_scenedata.m_filename = Parsing_String();		// �ϰ��� �ִ� �Լ��� ����� ���� String�� Int�� ��������.
			break;

		case TOKENR_SCENE_FIRSTFRAME:
			m_scenedata.m_firstframe = Parsing_NumberLong();
			break;

		case TOKENR_SCENE_LASTFRAME:
			m_scenedata.m_lastframe = Parsing_NumberLong();
			break;

		case TOKENR_SCENE_FRAMESPEED:
			m_scenedata.m_framespeed = Parsing_NumberLong();
			break;

		case TOKENR_SCENE_TICKSPERFRAME:
			m_scenedata.m_ticksperframe = Parsing_NumberLong();
			break;

		case TOKENR_SCENE_MESHFRAMESTEP:
			m_scenedata.m_meshframestep = Parsing_NumberLong();
			break;

		case TOKENR_SCENE_KEYFRAMESTEP:
			m_scenedata.m_keyframestep = Parsing_NumberLong();
			break;

		case TOKENR_SCENE_BACKGROUND_STATIC:
			m_scenedata.m_scene_background_static.x = Parsing_NumberFloat();
			m_scenedata.m_scene_background_static.y = Parsing_NumberFloat();
			m_scenedata.m_scene_background_static.z = Parsing_NumberFloat();
			break;

		case TOKENR_SCENE_AMBIENT_STATIC:
			m_scenedata.m_scene_ambient_static.x = Parsing_NumberFloat();
			m_scenedata.m_scene_ambient_static.y = Parsing_NumberFloat();
			m_scenedata.m_scene_ambient_static.z = Parsing_NumberFloat();
			break;

		case TOKENR_SCENE_ENVMAP:
		{
			// �׳� ���� ������ �о������ }�� ���ö�����
			while (nowtoken = m_Lexer->GetToken(m_TokenString), nowtoken != TOKEND_BLOCK_END) {
			}
		}
		break;

		//--------------------
		// MATERIAL_LIST
		//--------------------

		case TOKENR_MATERIAL_LIST:
			break;

		case TOKENR_MATERIAL_COUNT:
			m_MaterialCount = Parsing_NumberInt();
			break;

		case TOKENR_MATERIAL:
		{
			// Material ����
			m_MaterialVec->emplace_back(new ASEMaterial);
			m_NowMaterial = m_MaterialVec->back();
		}
		break;

		case TOKENR_MATERIAL_NAME:
			m_NowMaterial->m_material_name = Parsing_String();
			break;

		case TOKENR_MATERIAL_CLASS:
			m_NowMaterial->m_material_class = Parsing_String();
			break;

		case TOKENR_MATERIAL_AMBIENT:
			m_NowMaterial->m_material_ambient = Parsing_NumberVector3();
			break;

		case TOKENR_MATERIAL_DIFFUSE:
			m_NowMaterial->m_material_diffuse = Parsing_NumberVector3();
			break;

		case TOKENR_MATERIAL_SPECULAR:
			m_NowMaterial->m_material_specular = Parsing_NumberVector3();
			break;

		case TOKENR_MATERIAL_SHINE:
			m_NowMaterial->m_material_shine = Parsing_NumberFloat();
			break;

		case TOKENR_MATERIAL_SHINESTRENGTH:
			m_NowMaterial->m_material_shinestrength = Parsing_NumberFloat();
			break;

		case TOKENR_MATERIAL_TRANSPARENCY:
			m_NowMaterial->m_material_transparency = Parsing_NumberFloat();
			break;

		case TOKENR_MATERIAL_WIRESIZE:
			m_NowMaterial->m_material_wiresize = Parsing_NumberFloat();
			break;

		case TOKENR_MATERIAL_SHADING:
			break;

		case TOKENR_MATERIAL_XP_FALLOFF:
			m_NowMaterial->m_material_xp_falloff = Parsing_NumberFloat();
			break;

		case TOKENR_MATERIAL_SELFILLUM:
			m_NowMaterial->m_material_selfillum = Parsing_NumberFloat();
			break;

		case TOKENR_MATERIAL_FALLOFF:
			break;

		case TOKENR_MATERIAL_XP_TYPE:
			break;

		case TOKENR_MAP_DIFFUSE:
		{
			MaterialMap* newDiffuseMap = new MaterialMap;
			m_NowMaterial->m_map_diffuse = newDiffuseMap;
			m_NowMaterial->m_map_temp = newDiffuseMap;
		}
		break;

		case TOKENR_MAP_SPECULAR:
		{
			MaterialMap* newSpecMap = new MaterialMap;
			m_NowMaterial->m_map_specular = newSpecMap;
			m_NowMaterial->m_map_temp = newSpecMap;
		}
		break;

		case TOKENR_MAP_SHINE:
		{
			MaterialMap* newshineMap = new MaterialMap;
			m_NowMaterial->m_map_shine = newshineMap;
			m_NowMaterial->m_map_temp = newshineMap;
		}
		break;

		case TOKENR_MAP_BUMP:
		{
			MaterialMap* newBumpMap = new MaterialMap;
			m_NowMaterial->m_map_bump = newBumpMap;
			m_NowMaterial->m_map_temp = newBumpMap;
		}
		break;

		case TOKENR_MAP_NAME:
			m_NowMaterial->m_map_temp->m_map_name = Parsing_String();
			break;

		case TOKENR_MAP_CLASS:
			m_NowMaterial->m_map_temp->m_map_class = Parsing_String();
			break;

		case TOKENR_MAP_SUBNO:
			m_NowMaterial->m_map_temp->m_subno = Parsing_NumberInt();
			break;

		case TOKENR_MAP_AMOUNT:
			m_NowMaterial->m_map_temp->m_map_amount = Parsing_NumberFloat();
			break;

		case TOKENR_BITMAP:
			m_NowMaterial->m_map_temp->m_bitmap = Parsing_String();
			break;

		case TOKENR_MAP_TYPE:
			break;

		case TOKENR_UVW_U_OFFSET:
			m_NowMaterial->m_map_temp->m_uvw_u_offset = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_V_OFFSET:
			m_NowMaterial->m_map_temp->m_uvw_v_offset = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_U_TILING:
			m_NowMaterial->m_map_temp->m_u_tiling = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_V_TILING:
			m_NowMaterial->m_map_temp->m_v_tiling = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_ANGLE:
			m_NowMaterial->m_map_temp->m_uvw_angle = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_BLUR:
			m_NowMaterial->m_map_temp->m_uvw_blur = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_BLUR_OFFSET:
			m_NowMaterial->m_map_temp->m_uvw_blur_offset = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_NOUSE_AMT:
			m_NowMaterial->m_map_temp->m_uvw_noise_amt = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_NOISE_SIZE:
			m_NowMaterial->m_map_temp->m_uvw_noise_size = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_NOISE_LEVEL:
			m_NowMaterial->m_map_temp->m_uvw_noise_level = Parsing_NumberFloat();
			break;

		case TOKENR_UVW_NOISE_PHASE:
			m_NowMaterial->m_map_temp->m_uvw_noise_phase = Parsing_NumberFloat();
			break;

		case TOKENR_BITMAP_FILTER:
			break;

			//--------------------
			// GEOMOBJECT
			//--------------------

		case TOKENR_GROUP:
			//	�� ���� �׷� ����. �� ������ �̸��� ��Ʈ������ ������� �ϴµ�.
			break;

		case TOKENR_SHAPEOBJECT:
			m_MeshVec->emplace_back(new ASEMeshData);
			m_NowMesh = m_MeshVec->back();
			m_NowMesh->m_type = eObjectType_Shape;
			break;

		case TOKENR_HELPEROBJECT:
			// �޽� �ϳ� ���� (Helper Object�� ����ü�� ���� �־�� �� �� ������ �ϴ� ������.)
			m_MeshVec->emplace_back(new ASEMeshData);
			m_NowMesh = m_MeshVec->back();
			m_NowMesh->m_type = eObjectType_HelperObject;
			break;

		case TOKENR_GEOMOBJECT:
			// �޽� �ϳ� ����
			m_MeshVec->emplace_back(new ASEMeshData);
			m_NowMesh = m_MeshVec->back();
			m_NowMesh->m_type = eObjectType_Geomobject;
			break;

		case TOKENR_LIGHTOBJECT:
			m_MeshVec->emplace_back(new ASEMeshData);
			m_NowMesh = m_MeshVec->back();
			m_NowMesh->m_type = eObjectType_Light;
			break;

		case TOKENR_NODE_NAME:
			m_NowMesh->m_nodename = Parsing_String();

			if (m_IsFirstNodeName == false)
			{
				m_FirstNodeName = m_NowMesh->m_nodename;
				m_IsFirstNodeName = true;
			}

			// �� �޽��� Bone���� �ƴ���
			for (UINT i = 0; i < m_BoneNameVec.size(); i++)
			{
				if (m_BoneNameVec[i] == m_NowMesh->m_nodename)
				{
					m_NowMesh->m_IsBone = true;
					m_NowMesh->m_BoneIndex = i;
					break;
				}
			}

			break;

		case TOKENR_NODE_PARENT:
			m_NowMesh->m_nodeparent = Parsing_String();
			break;

			/// NODE_TM
		case TOKENR_NODE_TM:
			break;

		case TOKENR_INHERIT_POS:
			// ī�޶�� NodeTM�� �ι� ���´�. �ι�°��� ���� �ʴ´�.
			m_NowMesh->m_inherit_pos = Parsing_NumberVector3();
			break;

		case TOKENR_INHERIT_ROT:
			m_NowMesh->m_inherit_rot = Parsing_NumberVector3();
			break;

		case TOKENR_INHERIT_SCL:
			m_NowMesh->m_inherit_scl = Parsing_NumberVector3();
			break;

		case TOKENR_TM_ROW0:
			m_NowMesh->m_tm_row0 = Parsing_NumberVector3();
			break;

		case TOKENR_TM_ROW1:
			m_NowMesh->m_tm_row2 = Parsing_NumberVector3();
			break;

		case TOKENR_TM_ROW2:
			m_NowMesh->m_tm_row1 = Parsing_NumberVector3();
			break;

		case TOKENR_TM_ROW3:
			m_NowMesh->m_tm_row3 = Parsing_NumberVector3();
			break;

		case TOKENR_TM_POS:
		{
			SimpleMath::Vector3 pos = Parsing_NumberVector3();

			if (m_NowMesh->m_light_isloadTarget)
				m_NowMesh->m_light_TargetPos = pos;
			else
				m_NowMesh->m_tm_pos = pos;
		}
		break;
		case TOKENR_TM_ROTAXIS:
			m_NowMesh->m_tm_rotaxis = Parsing_NumberVector3();
			break;

		case TOKENR_TM_ROTANGLE:
			m_NowMesh->m_tm_rotangle = Parsing_NumberFloat();
			break;

		case TOKENR_TM_SCALE:
			m_NowMesh->m_tm_scale = Parsing_NumberVector3();
			break;

		case TOKENR_TM_SCALEAXIS:
			m_NowMesh->m_tm_scaleaxis = Parsing_NumberVector3();
			break;

		case TOKENR_TM_SCALEAXISANG:
			if (m_NowMesh->m_type == eObjectType_Light)
				m_NowMesh->m_light_isloadTarget = true;

			m_NowMesh->m_tm_scaleaxisang = Parsing_NumberFloat();
			break;

			/// Light
		case TOKENR_LIGHT_TYPE:
			m_NowMesh->m_light_type = Parsing_String();
			break;

		case TOKENR_LIGHT_SHADOWS:
			m_NowMesh->m_light_shadows = Parsing_String();
			break;

		case TOKENR_LIGHT_USELIGHT:
			m_NowMesh->m_light_uselight = Parsing_NumberInt();
			break;

		case TOKENR_LIGHT_SPOTSHAPE:
			m_NowMesh->m_light_spotshape = Parsing_String();
			break;

		case TOKENR_LIGHT_USEGLOBAL:
			m_NowMesh->m_light_useglobal = Parsing_NumberInt();
			break;

		case TOKENR_LIGHT_ABSMAPBIAS:
			m_NowMesh->m_light_absmapbias = Parsing_NumberInt();
			break;

		case TOKENR_LIGHT_OVERSHOOT:
			m_NowMesh->m_light_overshoot = Parsing_NumberInt();
			break;

		case TOKENR_LIGHT_SETTINGS:
			m_NowMesh->m_lightanimation = new Light_AnimationData;
			break;

		case TOKENR_TIMEVALUE:
			if (m_NowMesh->m_type == eObjectType_Light)
				m_NowMesh->m_lightanimation->m_timevalue = Parsing_NumberFloat();
			break;

		case TOKENR_LIGHT_COLOR:
			m_NowMesh->m_lightanimation->m_light_color = Parsing_NumberVector3();
			break;

		case TOKENR_LIGHT_INTENS:
			m_NowMesh->m_lightanimation->m_light_intens = Parsing_NumberFloat();
			break;

		case TOKENR_LIGHT_ASPECT:
			m_NowMesh->m_lightanimation->m_light_aspect = Parsing_NumberFloat();
			break;

		case TOKENR_LIGHT_HOTSPOT:
			m_NowMesh->m_lightanimation->m_light_hotspot = Parsing_NumberFloat();
			break;

		case TOKENR_LIGHT_FALLOFF:
			m_NowMesh->m_lightanimation->m_light_falloff = Parsing_NumberFloat();
			break;

		case TOKENR_LIGHT_TDIST:
			m_NowMesh->m_lightanimation->m_light_tdist = Parsing_NumberFloat();
			break;

		case TOKENR_LIGHT_MAPBIAS:
			m_NowMesh->m_lightanimation->m_light_mapbias = Parsing_NumberFloat();
			break;

		case TOKENR_LIGHT_MAPRANGE:
			m_NowMesh->m_lightanimation->m_light_maprange = Parsing_NumberFloat();
			break;

		case TOKENR_LIGHT_MAPSIZE:
			m_NowMesh->m_lightanimation->m_light_mapsize = Parsing_NumberInt();
			break;

		case TOKENR_LIGHT_RAYBIAS:
			m_NowMesh->m_lightanimation->m_light_raybias = Parsing_NumberFloat();
			break;

			/// Animation
		case TOKENR_TM_ANIMATION:
		{
			m_NowAnimationData = new MeshAnimationData;
			m_NowAnimationData->m_NodeName = m_NowMesh->m_nodename;

			m_NowMotionData->m_AnimationDataVec.push_back(m_NowAnimationData);

			// �ִϸ��̼��� �ִٰ� üũ
			m_NowMesh->m_IsAnimated = true;
		}
		break;

		case TOKENR_CONTROL_POS_TRACK:
			break;

		case TOKENR_CONTROL_ROT_TRACK:
			break;

		case TOKENR_CONTROL_POS_SAMPLE:
		{
			// KeyFrame
			int keyFrame = Parsing_NumberInt();
			m_NowAnimationData->m_PosKeyFrameVec.push_back(keyFrame);

			// Pos Sample
			//EMath::Vector3 posSample = Parsing_NumberVector3();
			//m_NowAnimationData->m_PosSampleVec.push_back(posSample);
		}
		break;

		case TOKENR_CONTROL_ROT_SAMPLE:
		{
			// KeyFrame
			int keyFrame = Parsing_NumberInt();

			m_NowAnimationData->m_RotKeyFrameVec.push_back(keyFrame);

			float rotSampleX = Parsing_NumberFloat();
			float rotSampleY = Parsing_NumberFloat();
			float rotSampleZ = Parsing_NumberFloat();
			float rotSampleW = Parsing_NumberFloat();

			SimpleMath::Quaternion rotSample = XMQuaternionRotationAxis(SimpleMath::Vector3(rotSampleX, rotSampleZ, rotSampleY), rotSampleW);

			// ���ʹϾ��� �����ؼ� �����ִ´�.
			size_t size = m_NowAnimationData->m_RotSampleVec.size();
			//if (size > 0)
			//	rotSample = XMQuaternionMultiply(m_NowAnimationData->m_RotSampleVec[size - 1], rotSample);

			//m_NowAnimationData->m_RotSampleVec.push_back(rotSample);
		}
		break;

		/// MESH

		case TOKENR_MESH:
			break;

		case TOKENR_MESH_NUMBONE:
			// �̰� �ִٸ� �̰��� Skinned Mesh��� ������ ���´�.
			m_NowMesh->m_IsSkinnedMesh = true;
			break;

		case TOKENR_MESH_NUMSKINWEIGHT:
			break;

		case TOKENR_MESH_NUMVERTEX:
			m_NowMesh->m_mesh_numvertex = Parsing_NumberInt();
			break;

		case TOKENR_MESH_NUMFACES:
			m_NowMesh->m_mesh_numfaces = Parsing_NumberInt();
			break;

			/// MESH_VERTEX_LIST

		case TOKENR_MESH_VERTEX_LIST:
			//
			// ���ؽ��� ������ ����־�� �ϴµ�
			// �̹� ���ͷ� ������ �� �����Ƿ� �׳� ������ �ȴ�.
			break;

		case TOKENR_MESH_VERTEX:
		{
			// ������ �Է�
			int index = Parsing_NumberInt();
			SimpleMath::Vector3 vertexPos = Parsing_NumberVector3();
			AVertex* newVertex = new AVertex();
			newVertex->m_pos = vertexPos;
			newVertex->m_indices = index;
			newVertex->m_prevIndex = index;

			m_NowMesh->m_meshvertex.push_back(newVertex);
		}
		break;

		/// Bone
		case TOKENR_SKIN_INITTM:
			break;

		case TOKENR_BONE_LIST:
			break;

		case TOKENR_BONE:
			break;

			//�� ������ ���� �̸��� �־�� �Ѵ�. ������ {�� �� �� �� �������Ƿ� �ӽ� ������ �����μ� �����ؾ߰���.
		case TOKENR_BONE_NAME:
		{
			std::string boneName = Parsing_String();
			m_BoneNameVec.push_back(boneName);
		}
		case TOKENR_BONE_PROPERTY:
			// �� ���� ABSOLUTE�� ������� �ϴµ�, �� ���� ����.
			break;

			// �������� TM_ROW0~3�� �����µ� ���� ���õ�..

		case TOKENR_MESH_WVERTEXS:
		{
			// ���� �ƴ� �޽� (��Ű�׵� �޽�)�� ã�´�.
			for (UINT i = 0; i < m_MeshVec->size(); i++)
			{
				for (UINT j = 0; j < m_BoneNameVec.size(); i++)
				{
					if (m_MeshVec->at(i)->m_nodename != m_BoneNameVec[j])
					{
						m_NowMesh = m_MeshVec->at(i);
						break;
					}
				}
			}
		}
		break;

		case TOKENR_MESH_WEIGHT:
		{
			wVertexIndex = Parsing_NumberInt();
		}
		break;

		case TOKENR_BONE_BLENGING_WEIGHT:
		{
			// ���� �ε���
			int boneIndex = Parsing_NumberInt();
			float weight = Parsing_NumberFloat();

			if (m_NowMesh != nullptr)
			{
				// Weight�� �����ϰ� vertex�� �ִ´�.
				Weight* newWeight = new Weight;
				newWeight->m_bone_number = boneIndex;
				newWeight->m_bone_weight = weight;

				m_NowMesh->m_meshvertex[wVertexIndex]->m_bone_blending_weight.push_back(newWeight);
			}
		}
		break;

		/// MESH_FACE_LIST
		case TOKENR_MESH_FACE_LIST:
			//
			break;

		case TOKENR_MESH_FACE:
		{
			Face* newFace = new Face();

			// Face�� ��ȣ�ε�...
			int num = Parsing_NumberInt();
			//Parsing_String();
			// A:�� �а�
			std::string s = Parsing_String();
			newFace->m_vertexindex[0] = Parsing_NumberInt();
			// B:
			Parsing_String();
			newFace->m_vertexindex[2] = Parsing_NumberInt();
			// C:
			Parsing_String();
			newFace->m_vertexindex[1] = Parsing_NumberInt();

			/// (�ڿ� ������ �� ������ default�� ���� ��ŵ�� ���̴�.)
			/// ......

			// ���Ϳ� �־��ش�.
			m_NowMesh->m_meshface.push_back(newFace);
		}
		break;

		case TOKENR_MESH_NUMTVERTEX:
			break;

			/// MESH_TVERTLIST

		case TOKENR_MESH_TVERTLIST:
			break;

		case TOKENR_MESH_TVERT:
		{
			// ���ؽ��� �ε����� �����µ� ������ ������ �����Ƿ� ������.
			// ���ο� TVertex�� ���� ���Ϳ� �ִ´�
			int index = Parsing_NumberInt();

			float _u = Parsing_NumberFloat();
			float _v = 1.0f - Parsing_NumberFloat();
			float _w = Parsing_NumberFloat();

			m_NowMesh->m_mesh_tvertex.emplace_back(new COneTVertex(_u, _v));
		}

		break;
		case TOKENR_MESH_NUMTVFACES:
			break;

			/// TFACE

		case TOKENR_MESH_TFACE:
		{
			int index = Parsing_NumberInt();

			m_NowMesh->m_meshface[index]->m_TFace[0] = Parsing_NumberInt();
			m_NowMesh->m_meshface[index]->m_TFace[2] = Parsing_NumberInt();
			m_NowMesh->m_meshface[index]->m_TFace[1] = Parsing_NumberInt();
		}
		break;

		/// MESH_NORMAL

		case TOKENR_MESH_NORMALS:
			break;

		case TOKENR_MESH_FACENORMAL:
			// Face�� �ε���
			faceIndex = Parsing_NumberInt();

			// Face�� �븻
			m_NowMesh->m_meshface[m_FaceIndex]->m_normal.x = Parsing_NumberFloat();
			m_NowMesh->m_meshface[m_FaceIndex]->m_normal.z = Parsing_NumberFloat();
			m_NowMesh->m_meshface[m_FaceIndex]->m_normal.y = Parsing_NumberFloat();

			m_FaceVertexIndex = 0;
			//m_NowMesh->m_meshface[faceIndex]->m_normal = Parsing_NumberVector3();

			break;

		case TOKENR_MESH_VERTEXNORMAL:
		{
			int vni = 0;

			switch (m_FaceVertexIndex)
			{
			case 0:
				vni = 0;
				break;
			case 1:
				vni = 2;
				break;
			case 2:
				vni = 1;
				break;
			}

			int index = Parsing_NumberInt();
			SimpleMath::Vector3 newNormal = Parsing_NumberVector3();

			m_NowMesh->m_meshface[m_FaceIndex]->m_VertexNormal[vni] = newNormal;
			m_FaceVertexIndex++;
		}
		break;

		case TOKEND_END:
			// �Ƹ��� �̰� ������ ���� ��Ÿ�������ΰ� ������. while�� Ż���ؾ� �ϴµ�?

			//AfxMessageBox("������ ���� �� �� �����ϴ�!", MB_OK, NULL);
			TRACE("TRACE: �Ľ���: ������ ���� �ý��ϴ�!\n");
			return;

			break;

			/// ���� �ƹ��͵� �ش����� ������
		default:
			// �ƹ��͵� ���� �ʴ´�.
			break;

		}	// switch()


		///-----------------------------------------------
		/// ���� �ڵ�.
		i++;
		if (i > 1000000)
		{
			// ������ 1000000���̻��̳� �� ������ ����. (�����Ͱ� 100000���� �ƴ��̻�)
			// ���� 1000000�̻� ���Ҵٸ� Ȯ���� ���� ������ �ִ� ���̹Ƿ�
			TRACE("������ �鸸�� ���ҽ��ϴ�!");
			return;
		}
		/// ���� �ڵ�.
		///-----------------------------------------------
	}
}

int ASEParser::Parsing_NumberLong()
{
	LONG			token;
	LONG			tempNumber;

	token = m_Lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
	tempNumber = strtoul(m_TokenString, NULL, 10);

	return			tempNumber;
}

float ASEParser::Parsing_NumberFloat()
{
	LONG			token;
	float			tempNumber;

	token = m_Lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
	tempNumber = (float)atof(m_TokenString);

	return			tempNumber;
}

LPSTR ASEParser::Parsing_String()
{
	/// ��m_TokenString ( char[255] ) �̱� ������ CString�� ������ ���� ���Ŷ� �����ߴµ�, �������� CString�� �� ������� �� ����. �˾Ƽ� �޾Ƶ��̴µ�?
	m_Lexer->GetToken(m_TokenString);

	return m_TokenString;
}

int ASEParser::Parsing_NumberInt()
{
	LONG			token;
	int				tempNumber;

	token = m_Lexer->GetToken(m_TokenString);	//ASSERT(token == TOKEND_NUMBER);
	tempNumber = (int)atoi(m_TokenString);

	return			tempNumber;
}

DirectX::SimpleMath::Vector3 ASEParser::Parsing_NumberVector3()
{
	LONG				token;
	SimpleMath::Vector3	tempVector3;

	token = m_Lexer->GetToken(m_TokenString);
	tempVector3.x = (float)atof(m_TokenString);
	token = m_Lexer->GetToken(m_TokenString);
	tempVector3.z = (float)atof(m_TokenString);
	token = m_Lexer->GetToken(m_TokenString);
	tempVector3.y = (float)atof(m_TokenString);

	return			tempVector3;		// ����ƽ ������ ���۷������ٴ� �� ������ ����.
}
