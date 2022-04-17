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

	/// 0) 파일을 로드한다.
	if (!m_Lexer->Open(path + fileName))
	{
		TRACE("파일을 여는 중에 문제가 발생했습니다!");
		return FALSE;
	}

	m_MeshVecMap[fileName] = new std::vector<ASEMeshData*>;
	m_MeshVec = m_MeshVecMap[fileName];

	m_MaterialVecMap[fileName] = new std::vector<ASEMaterial*>;
	m_MaterialVec = m_MaterialVecMap[fileName];

	m_NowMotionData = new Motion;

	// 비재귀로 ASE파일을 파싱한다.
	ParseASE();

	for (UINT i = 0; i < m_MeshVec->size(); i++)
	{
		//ConvertAll(m_MeshVec->at(i));
		OptimiseMesh(m_MeshVec->at(i));
		ConvertVertexToLocalCoordinate(m_MeshVec->at(i));
	}

	m_BoneNameVec.clear();

	m_IsFirstNodeName = false;

	/// 모션 데이터
	// 애니메이션 데이터가 없다면 이 부분은 넘어간다.
	if (m_NowMotionData->m_AnimationDataVec.size() <= 0)
	{
		return TRUE;
	}

	// 첫번째 노드의 이름 저장해놓음 (오브젝트 구분용)
	m_NowMotionData->m_ObjectNodeName = m_FirstNodeName;

	std::string motionName;

	// 파일 이름의 끝에서 _가 몇번째인지 찾는다.
	size_t findIndex = fileName.find_last_of("_");

	// _가 없다면 Default 이름으로 설정
	if (findIndex == -1)
	{
		motionName = "None";
	}
	else
	{
		findIndex++;

		// 모션 이름과 확장자만 남김
		motionName = fileName.substr(findIndex, fileName.size() - findIndex);

		// 확장자 제거
		motionName = motionName.substr(0, motionName.size() - 4);
	}

	// 모션의 이름을 저장
	m_NowMotionData->m_MotionName = motionName;

	/// 같은 오브젝트인지 체크해야함
	std::map<std::string, Motion*> motionMap = m_ObjectAnimationMap[m_FirstNodeName];

	// 같은 이름의 오브젝트가 맵에 없다면
	if (motionMap.size() <= 0)
	{
		motionMap[motionName] = m_NowMotionData;
		m_ObjectAnimationMap[m_FirstNodeName] = motionMap;
	}
	// 이미 있다면 기존 벡터에 넣는다. (같은 오브젝트이므로)
	else
	{
		m_ObjectAnimationMap[m_FirstNodeName][motionName] = m_NowMotionData;
	}

	return TRUE;
}

bool ASEParser::ConvertAll(ASEMeshData* pMesh)
{
	// 버텍스들은 일단 모두 복사
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

	// 나머지는 face를 기준으로 한 w인덱스로 찾아야 함
	size_t _faceCount = pMesh->m_meshface.size();
	TRACE("FaceCount : %d", _faceCount);

	for (unsigned int i = 0; i < pMesh->m_meshface.size(); i++)
	{
		TRACE("FaceNum : %d / %d\n", i, _faceCount);

		for (int j = 0; j < 3; j++)
		{
			AVertex* _nowVertex = pMesh->m_opt_vertex[pMesh->m_meshface[i]->m_vertexindex[j]];

			// veretex기준으로 버텍스의 노말을 넣어준다.
			_nowVertex->m_normal.x = pMesh->m_meshface[i]->m_VertexNormal[j].x;
			_nowVertex->m_normal.y = pMesh->m_meshface[i]->m_VertexNormal[j].y;
			_nowVertex->m_normal.z = pMesh->m_meshface[i]->m_VertexNormal[j].z;
		}
	}

	// 인덱스는 그냥 복사
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
	static bool isVertexCreateEnd = false;		// for문 탈출용 변수

	// 6중 for문을 돌면서 vertex 정보 입력, vertex 생성, face index 세팅을 모두 한 번에 한다.
	// 2중 for문 x 3 이라고 보면 된다.
	// 첫번째 2중 for문 : face vector를 순회하면서 face에 저장되있는 vertex정보를 넣고 index를 세팅한다.
	// 두번째 2중 for문 : 현재 face index(i) 보다 작은 face들을 순회하면서 생성할 vertex가 있는지 검사한다.
	// 세번째 2중 for문 : 현재 face index(i) 보다 큰 face들을 순회하면서 생성된 vertex의 새로운 index를 넣어준다.
	for (UINT i = 0; i < mesh->m_meshface.size(); i++)
	{
		IndexList* indexList = new IndexList;

		for (UINT j = 0; j < 3; j++)
		{
			// normal 비교
			int checkIndex = mesh->m_meshface[i]->m_vertexindex[j];			// 현재 face의 j번째 인덱스
			SimpleMath::Vector3 checkNormal = mesh->m_meshface[i]->m_VertexNormal[j];	// 현재 face의 j번째 노말

			// u,v 비교
			float checkU = -1;		// 현재 face의 j번째 tVertex 인덱스의 u 값
			float checkV = -1;		// 현재 face의 j번째 tVertex 인덱스의 v 값

			// tvertex 정보가 없는 ase 파일을 파싱할 때 예외처리
			if (mesh->m_mesh_tvertex.size() > 0)
			{
				checkU = mesh->m_mesh_tvertex[mesh->m_meshface[i]->m_TFace[j]]->m_u;
				checkV = mesh->m_mesh_tvertex[mesh->m_meshface[i]->m_TFace[j]]->m_v;
			}

			// 이전의 Face들과 비교한다.
			for (UINT k = 0; k < i; k++)
			{
				// vertex의 index, normal, uv 3개 모두 비교
				for (UINT l = 0; l < 3; l++)
				{
					// 인덱스가 같고
					if (checkIndex == mesh->m_meshface[k]->m_vertexindex[l])
					{
						// 노말이 다르면
						if (checkNormal != mesh->m_meshface[k]->m_VertexNormal[l])
						{
							// 새로운 버텍스 생성
							AVertex* newVertex = new AVertex;

							// pos, uv는 같게, weight도 같게
							newVertex->m_pos = mesh->m_meshvertex[checkIndex]->m_pos;
							newVertex->u = mesh->m_meshvertex[checkIndex]->u;
							newVertex->v = mesh->m_meshvertex[checkIndex]->v;
							newVertex->m_bone_blending_weight = mesh->m_meshvertex[checkIndex]->m_bone_blending_weight;

							// normal은 다르게
							newVertex->m_normal = checkNormal;

							// 인덱스도 새로 세팅
							newVertex->m_indices = mesh->m_meshvertex.size();
							mesh->m_meshvertex.push_back(newVertex);

							// 인덱스 다시 이어주기
							for (UINT q = i; q < mesh->m_meshface.size(); q++)
							{
								for (UINT w = j; w < 3; w++)
								{
									// 검사할 인덱스
									int checkIndexForChange = mesh->m_meshface[q]->m_vertexindex[w];

									// 검사할 노말 값
									SimpleMath::Vector3 checkNormalForChange = mesh->m_meshface[q]->m_VertexNormal[w];

									// 인덱스와 u,v가 모두 같으면 새로운 vertex의 인덱스 정보를 넣어준다.
									if (checkIndex == checkIndexForChange &&
										checkNormal == checkNormalForChange)
									{
										mesh->m_meshface[q]->m_vertexindex[w] = newVertex->m_indices;
									}
								}
							}

							// for문 탈출용
							isVertexCreateEnd = true;
							break;
						}

						// u,v가 다르면
						if (mesh->m_mesh_tvertex.size() > 0 &&
							checkU != mesh->m_mesh_tvertex[mesh->m_meshface[k]->m_TFace[l]]->m_u &&
							checkV != mesh->m_mesh_tvertex[mesh->m_meshface[k]->m_TFace[l]]->m_v)
						{
							AVertex* newVertex = new AVertex;

							// pos, normal은 같게, weight도 같게
							newVertex->m_pos = mesh->m_meshvertex[checkIndex]->m_pos;
							newVertex->m_normal = mesh->m_meshvertex[checkIndex]->m_normal;
							newVertex->m_bone_blending_weight = mesh->m_meshvertex[checkIndex]->m_bone_blending_weight;

							// u, v는 다르게
							newVertex->u = checkU;
							newVertex->v = checkV;

							// 인덱스도 새로 세팅
							newVertex->m_indices = mesh->m_meshvertex.size();
							mesh->m_meshvertex.push_back(newVertex);

							// 인덱스 다시 이어주기 (지금 face index의 다음부터만 검사)
							for (UINT q = i; q < mesh->m_meshface.size(); q++)
							{
								for (UINT w = j; w < 3; w++)
								{
									// 검사할 인덱스
									int checkIndexForChange = mesh->m_meshface[q]->m_vertexindex[w];

									// 검사할 u, v값
									float checkUForChange = mesh->m_mesh_tvertex[mesh->m_meshface[q]->m_TFace[w]]->m_u;
									float checkVForChange = mesh->m_mesh_tvertex[mesh->m_meshface[q]->m_TFace[w]]->m_v;

									// 인덱스와 u,v가 모두 같으면 새로운 vertex의 인덱스 정보를 넣어준다.
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

				// for문 탈출
				if (isVertexCreateEnd)
				{
					isVertexCreateEnd = false;
					break;
				}
			}

			//현재 face의 j번째 vertex
			AVertex* _nowVertex = mesh->m_meshvertex[mesh->m_meshface[i]->m_vertexindex[j]];

			//face를 순회하면서 vertex에 노말 값을 넣어준다.
			_nowVertex->m_normal.x = mesh->m_meshface[i]->m_VertexNormal[j].x;
			_nowVertex->m_normal.y = mesh->m_meshface[i]->m_VertexNormal[j].y;
			_nowVertex->m_normal.z = mesh->m_meshface[i]->m_VertexNormal[j].z;

			// u, v 값을 넣어준다.
			if (mesh->m_mesh_tvertex.size() > 0)
			{
				_nowVertex->u = mesh->m_mesh_tvertex[mesh->m_meshface[i]->m_TFace[j]]->m_u;
				_nowVertex->v = mesh->m_mesh_tvertex[mesh->m_meshface[i]->m_TFace[j]]->m_v;
			}

			//인덱스를 넣어준다.
			indexList->index[j] = mesh->m_meshface[i]->m_vertexindex[j];
		}

		// 3개의 인덱스가 다 넣어졌으면 vector에 넣는다.
		mesh->m_opt_index.push_back(indexList);
	}

	// 버텍스를 그대로 복사한다.
	mesh->m_opt_vertex.assign(mesh->m_meshvertex.begin(), mesh->m_meshvertex.end());

	return FALSE;
}

bool ASEParser::OptimiseMesh3(ASEMeshData* pMesh)
{
	// 2021.04.12
	// 구현 제거 (예시용)
	// 이것이 메시가 아니라면 리턴

	unsigned int _faceCount = pMesh->m_meshface.size();
	pMesh->m_opt_index.resize(pMesh->m_meshface.size());

	for (unsigned int i = 0; i < pMesh->m_meshface.size(); i++)
	{
		pMesh->m_opt_index[i] = new IndexList;

		for (int j = 0; j < 3; j++)
		{
			// 1. Vertex를 조합한다
			// 단, 데이터가 없으면 조합하지 않는다
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

			// 2. 지금까지 조합된 Vertex중에 같은 Vertex가 있는지 검색한다
			int index = 0;
			for (unsigned int vlistindex = 0; vlistindex < pMesh->m_opt_vertex.size(); vlistindex++, index++)
			{
				// 같은 버텍스가 있으면 지금 조합된것을 삭제하고 루프를 빠져나감
				if (pMesh->m_opt_vertex[index]->m_pos == (pvertex)->m_pos &&
					pMesh->m_opt_vertex[index]->m_normal == (pvertex)->m_normal &&
					pMesh->m_opt_vertex[index]->u == (pvertex)->u &&
					pMesh->m_opt_vertex[index]->v == (pvertex)->v)
				{
					/// 잠시 삭제
					///TRACE("같은 버텍스 삭제 : %d\n", index);
					delete pvertex;
					pvertex = nullptr;
					break;
				}
			}

			// vertex가 삭제되지 않았다면 같은 vertex가 없다는 것이므로
			// 리스트에 추가한 뒤, 인덱스를 넣어준다 번호는 index
			if (pvertex != NULL)
			{
				pMesh->m_opt_vertex.push_back(pvertex);
				//m_opt_index[i][j] = index;
				pMesh->m_opt_index[i]->index[j] = index;
			}
			else // vertex가 삭제되었을 때 -> 같은 Vertex가 있다는 것, 그에 해당하는 인덱스도 index다
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
	// Node TM 및 Node TM의 역행렬을 구한다.
	XMMATRIX scale = XMMatrixScaling(mesh->m_tm_scale.x, mesh->m_tm_scale.y, mesh->m_tm_scale.z);
	SimpleMath::Vector3 scaleAxis = mesh->m_tm_scaleaxis;
	float scaleAng = mesh->m_tm_scaleaxisang;

	// Negative Scale인지 확인
	SimpleMath::Vector3 row0 = mesh->m_tm_row0;
	SimpleMath::Vector3 row1 = mesh->m_tm_row1;
	SimpleMath::Vector3 row2 = mesh->m_tm_row2;

	// [ Negative Scale 확인 방법 ]
	// row0, row1, row2가 각각 x,y,z축을 나타내므로 이 중 두 축을 '외적'한 값이 다른 한 축과 방향이 반대인지 혹은 같은지를 확인하면된다.
	// -> 외적 한 값과 다른 한 축을 내적한다. (음수일 경우 축이 반대, 양수일 경우 축이 같은 방향)
	bool isNegativeScale = false;
	SimpleMath::Vector3 crossResult = row0.Cross(row1);
	float checkNegativeScaleResult = crossResult.Dot(row2);

	// 음수면 축이 반대이다.
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

	// NodeTm 역행렬 구하기
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
		// Pos의 최대, 최소 저장 (Collider Box 생성을 위해)
		XMVECTOR P = XMLoadFloat3(&mesh->m_opt_vertex[i]->m_pos);

		vMin = XMVectorMin(vMin, P);
		vMax = XMVectorMax(vMax, P);

		// 버텍스들을 world -> local로 보낸다.
		mesh->m_opt_vertex[i]->m_pos = DirectX::XMVector3Transform(mesh->m_opt_vertex[i]->m_pos, worldInverse);

		// 노말 값을 뒤집어준다.
		//if (isNegativeScale)
		//	mesh->m_opt_vertex[i]->m_normal *= -1;
	}

	// Pos 최대, 최소 저장
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

	/// 이것을 하면 한 개의 토큰을 읽고, 그 종류와 내용을 알 수 있다.
	while (nowtoken = m_Lexer->GetToken(m_TokenString), nowtoken != TOKEND_END)
	{
		// 일단 한 개의 토큰을 읽고, 그것이 괄호 닫기가 아니라면.
		// 넘어온 토큰에 따라 처리해준다.

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
			Parsing_String();	// 그냥 m_TokenString에 읽어버리는 역할 뿐.
			//AfxMessageBox( m_TokenString, NULL, NULL);		/// 임시로 코멘트를 출력해본다
			break;

			//--------------------
			// SCENE
			//--------------------

		case TOKENR_SCENE:
			break;

		case TOKENR_SCENE_FILENAME:
			m_scenedata.m_filename = Parsing_String();		// 일관성 있는 함수의 사용을 위해 String과 Int도 만들어줬다.
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
			// 그냥 안의 내용을 읽어버린다 }가 나올때까지
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
			// Material 생성
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
			//	한 개의 그룹 시작. 이 다음에 이름이 스트링으로 나오기는 하는데.
			break;

		case TOKENR_SHAPEOBJECT:
			m_MeshVec->emplace_back(new ASEMeshData);
			m_NowMesh = m_MeshVec->back();
			m_NowMesh->m_type = eObjectType_Shape;
			break;

		case TOKENR_HELPEROBJECT:
			// 메쉬 하나 생성 (Helper Object용 구조체가 따로 있어야 할 것 같지만 일단 만들어본다.)
			m_MeshVec->emplace_back(new ASEMeshData);
			m_NowMesh = m_MeshVec->back();
			m_NowMesh->m_type = eObjectType_HelperObject;
			break;

		case TOKENR_GEOMOBJECT:
			// 메쉬 하나 생성
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

			// 이 메쉬가 Bone인지 아닌지
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
			// 카메라는 NodeTM이 두번 나온다. 두번째라면 넣지 않는다.
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

			// 애니메이션이 있다고 체크
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

			// 쿼터니언을 누적해서 갖고있는다.
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
			// 이게 있다면 이것은 Skinned Mesh라고 단정을 짓는다.
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
			// 버텍스의 값들을 집어넣어야 하는데
			// 이미 벡터로 선언이 돼 있으므로 그냥 넣으면 된다.
			break;

		case TOKENR_MESH_VERTEX:
		{
			// 데이터 입력
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

			//이 다음에 본의 이름을 넣어야 한다. 하지만 {를 한 개 더 열었으므로 임시 포인터 변수로서 보관해야겠지.
		case TOKENR_BONE_NAME:
		{
			std::string boneName = Parsing_String();
			m_BoneNameVec.push_back(boneName);
		}
		case TOKENR_BONE_PROPERTY:
			// 이 다음 ABSOLUTE가 나오기는 하는데, 쓸 일이 없다.
			break;

			// 다음에는 TM_ROW0~3이 나오는데 역시 무시됨..

		case TOKENR_MESH_WVERTEXS:
		{
			// 본이 아닌 메쉬 (스키닝된 메쉬)를 찾는다.
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
			// 본의 인덱스
			int boneIndex = Parsing_NumberInt();
			float weight = Parsing_NumberFloat();

			if (m_NowMesh != nullptr)
			{
				// Weight를 생성하고 vertex에 넣는다.
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

			// Face의 번호인데...
			int num = Parsing_NumberInt();
			//Parsing_String();
			// A:를 읽고
			std::string s = Parsing_String();
			newFace->m_vertexindex[0] = Parsing_NumberInt();
			// B:
			Parsing_String();
			newFace->m_vertexindex[2] = Parsing_NumberInt();
			// C:
			Parsing_String();
			newFace->m_vertexindex[1] = Parsing_NumberInt();

			/// (뒤에 정보가 더 있지만 default에 의해 스킵될 것이다.)
			/// ......

			// 벡터에 넣어준다.
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
			// 버텍스의 인덱스가 나오는데 어차피 순서와 같으므로 버린다.
			// 새로운 TVertex를 만들어서 벡터에 넣는다
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
			// Face의 인덱스
			faceIndex = Parsing_NumberInt();

			// Face의 노말
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
			// 아마도 이건 파일의 끝이 나타났을때인것 같은데. while을 탈출해야 하는데?

			//AfxMessageBox("파일의 끝을 본 것 같습니다!", MB_OK, NULL);
			TRACE("TRACE: 파싱중: 파일의 끝을 봤습니다!\n");
			return;

			break;

			/// 위의 아무것도 해당하지 않을때
		default:
			// 아무것도 하지 않는다.
			break;

		}	// switch()


		///-----------------------------------------------
		/// 안전 코드.
		i++;
		if (i > 1000000)
		{
			// 루프를 1000000번이상이나 돌 이유가 없다. (데이터가 100000개가 아닌이상)
			// 만약 1000000이상 돌았다면 확실히 뭔가 문제가 있는 것이므로
			TRACE("루프를 백만번 돌았습니다!");
			return;
		}
		/// 안전 코드.
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
	/// ※m_TokenString ( char[255] ) 이기 때문에 CString에 넣으면 에러 날거라 생각했는데, 생각보다 CString은 잘 만들어진 것 같다. 알아서 받아들이는데?
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

	return			tempVector3;		// 스태틱 변수의 레퍼런스보다는 값 전달을 하자.
}
