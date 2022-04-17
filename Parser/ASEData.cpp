#include "ASEData.h"

namespace ASEParserData
{
	AVertex::AVertex()
		: m_pos(), m_normal(), m_isnormalset(false)
		, u(0.0f), v(0.0f), m_istextureset(false)
		, m_bw1(0.0f), m_bw2(0.0f), m_bw3(0.0f), m_indices(0), m_indicesPointer(nullptr)
	{

	}

	Face::Face()
	{
		m_TFace[0] = 0;
		m_TFace[1] = 0;
		m_TFace[2] = 0;

		m_vertexindex[0] = 0;
		m_vertexindex[1] = 0;
		m_vertexindex[2] = 0;

		m_normal = SimpleMath::Vector3(0, 0, 0);
	}

	COneTVertex::COneTVertex()
	{
		m_u = 0;
		m_v = 0;
		m_w = 0;
	}

	ShapeLine::ShapeLine()
	{
		m_line_number = 0;			// ���° �����ΰ�?
		m_shape_closed = false;		// ���� Shape�ΰ�?
		m_shape_vertexcount = 0;	// �� ���� Vertex�� �ִ°�?
		//m_shapevertex;			// ���ؽ����� vector
	}

	Scenedata::Scenedata()
	{
		//m_filename;		CString �ʱ�ȭ �ʿ����?
		m_firstframe = 0;
		m_lastframe = 0;
		m_framespeed = 0;
		m_ticksperframe = 0;
		m_meshframestep = 0;
		m_keyframestep = 0;
		m_scene_background_static = SimpleMath::Vector3(0, 0, 0);
		m_scene_ambient_static = SimpleMath::Vector3(0, 0, 0);
	}

	Scenedata::~Scenedata()
	{

	}

	MaterialMap::MaterialMap()
	{
		//CString m_map_name;
		//CString m_map_class;
		m_subno = 0;
		m_map_amount = 0;
		//CString m_bitmap;
		m_map_type = 0;
		m_uvw_u_offset = 0;
		m_uvw_v_offset = 0;
		m_u_tiling = 0;
		m_v_tiling = 0;
		m_uvw_angle = 0;
		m_uvw_blur = 0;
		m_uvw_blur_offset = 0;
		m_uvw_noise_amt = 0;
		m_uvw_noise_size = 0;
		m_uvw_noise_level = 0;
		m_uvw_noise_phase = 0;
		m_bitmap_filter = 0;
	}

	ASEMaterial::ASEMaterial()
	{
		m_materialnumber = 0;	// Material�� ��ȣ. �̰��� �������� ������Ʈ���� �����Ѵ�.
		//CString m_material_name;
		//CString m_material_class;
		m_material_ambient = SimpleMath::Vector3(0, 0, 0);
		m_material_diffuse = SimpleMath::Vector3(0, 0, 0);
		m_material_specular = SimpleMath::Vector3(0, 0, 0);
		m_material_shine = 0.0f;
		m_material_shinestrength = 0.0f;
		m_material_transparency = 0.0f;
		m_material_wiresize = 0;
		m_material_shading = 0;
		m_material_xp_falloff = 0.0f;
		m_material_selfillum = 0.0f;
		m_material_falloff = 0.0f;
		m_material_xp_type = 0;

		m_numsubmtls = 0;

		m_istextureexist = false;

		m_map_bump = nullptr;
		m_map_diffuse = nullptr;
		m_map_generic = nullptr;
		m_map_shine = nullptr;
		m_map_specular = nullptr;
		m_map_temp = nullptr;
		m_submaterial = nullptr;
	}

	ASEMaterial::~ASEMaterial()
	{

	}

	Light_AnimationData::Light_AnimationData()
	{
		// Light Setting
		m_timevalue = -1;			// �̰� �ִϸ��̼��� ���°���� �����̴�. �׷��� �ð��� -1�� ��찡 ������ ��¼��?
		m_light_color = SimpleMath::Vector3(1.0f, 1.0f, 1.0f);
		m_light_intens = 0.0f;
		m_light_aspect = 0.0f;
		m_light_hotspot = 0.0f;		///
		m_light_falloff = 0.0f;		/// �� �ΰ����� SpotLight�� ���
		m_light_tdist = 0.0f;
		m_light_mapbias = 0.0f;
		m_light_maprange = 0.0f;
		m_light_mapsize = 0;
		m_light_raybias = 0.0f;
	}

	ASEMeshData::ASEMeshData()
	{
		m_type = 0;
		m_IsAnimated = false;
		//CString m_nodename;				// CString�� �ڵ����� �ʱ�ȭ �����ִ°�?
		//CString m_nodeparent;

		//CString m_meshname;
		m_inherit_pos = SimpleMath::Vector3(0, 0, 0);
		m_inherit_rot = SimpleMath::Vector3(0, 0, 0);
		m_inherit_scl = SimpleMath::Vector3(0, 0, 0);
		m_tm_row0 = SimpleMath::Vector3(0, 0, 0);
		m_tm_row1 = SimpleMath::Vector3(0, 0, 0);
		m_tm_row2 = SimpleMath::Vector3(0, 0, 0);
		m_tm_row3 = SimpleMath::Vector3(0, 0, 0);
		m_tm_pos = SimpleMath::Vector3(0, 0, 0);
		m_tm_rotaxis = SimpleMath::Vector3(0, 0, 0);
		m_tm_rotangle = 0;
		m_tm_scale = SimpleMath::Vector3(0, 0, 0);
		m_tm_scaleaxis = SimpleMath::Vector3(0, 0, 0);
		m_tm_scaleaxisang = 0;

		m_timevalue = 0;
		m_numbone = 0;						/// ���� ����. �̰��� �ִٸ� ��Ű�� ������Ʈ�� �����Ѵ�.
		m_numskinweight = 0;				/// ��Ų ����ġ. ���Ҹ���?
		m_mesh_numvertex = 0;
		m_mesh_numfaces = 0;

		//Vector3* m_meshvertex;			// �̰� ���ؽ�
		//Vector3* m_meshface;				// �̰� ���̽�(���ؽ� �ε���, �븻)

		m_mesh_numtvertex = 0;

		//m_mesh_tverlist			// �ؽ����� ���ؽ� ����Ʈ
		m_mesh_tvfaces = 0;
		//m_mesh_tfacelist =		// �ؽ����� ���̽� ����Ʈ
		m_mesh_numcvertex = 0;

		m_mesh_facenormal = 0;		// Face��	normal
		m_mesh_vertexnormal = 0;	// Vertex�� normal

		m_prop_motionblur = 0;
		m_prop_castshadow = 0;
		m_prop_recvshadow = 0;
		m_wireframe_color = SimpleMath::Vector3(0, 0, 0);

		m_material_ref = 0;

		m_shape_linecount = 0;
		m_wvertex = nullptr;

		///----------------------------------
		/// Camera
		///----------------------------------
		//CString m_camera_type;

		// CAMERA_SETTINGS
		m_timevalue = 0;
		m_camera_near = 0.0f;
		m_camera_fov = 0.0f;
		m_camera_tdist = 0.0f;

		m_camera_isloadTarget = false;
		m_camera_far = 0;

		///----------------------------------
		/// Light
		///----------------------------------
		//CString m_light_type;
		// �̳༮�� Target�� �ֱ�..

		// Light Property
		//CString m_light_shadows;
		m_light_uselight = false;
		//CString m_spotshape;
		m_light_useglobal = false;
		m_light_absmapbias = false;
		m_light_overshoot = false;

		// Light Setting
		m_light_isloadTarget = false;
		m_light_isAnimate = false;

		m_lightanimation = nullptr;

		///----------------------------------
		/// Skinning Object
		///----------------------------------
		m_bone = nullptr;
		// SKIN_INITTM�� �����ϰ�
		// TM_ROW0~3
		// BONE_LIST
		m_IsBone = false;			// ��Ű�� ������Ʈ�ΰ�?
		m_IsSkinnedMesh = false;
		m_BoneIndex = 0;
		//vector<CBone*> m_vector_bone_list;				// �̰� �׳� ��ȣ�� �̸� �����̴�. �� �ȿ� �����͸� �־���Ƶ� �����ڱ���(�����ϱ� ���ϰ� - �׷��� ������ �Ź� �̸����� �˻��� �ؾߵ�����. �ƴϸ� �ε����� ������ ��� �ð���� ������ �Ҽ���?)
		//vector<CWeight_Vertex*> m_vector_wvertexs;		// ���ؽ� �ϳ��� �޶�پ��ִ� ����ġ��
	}

	ASEMeshData::~ASEMeshData()
	{
		// �Ҹ��ڿ��� �����Ǿ��� �޽õ����� (���ؽ�����Ʈ, ���̽� ����Ʈ) �����ؾ���.
	}
}
