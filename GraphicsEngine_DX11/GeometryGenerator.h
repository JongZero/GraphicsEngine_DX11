//***************************************************************************************
// GeometryGenerator.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Defines a static class for procedurally generating the geometry of 
// common mathematical objects.
//
// All triangles are generated "outward" facing.  If you want "inward" 
// facing triangles (for example, if you want to place the camera inside
// a sphere to simulate a sky), you will need to:
//   1. Change the Direct3D cull mode or manually reverse the winding order.
//   2. Invert the normal.
//   3. Update the texture coordinates and tangent vectors.
//***************************************************************************************

#ifndef GEOMETRYGENERATOR_H
#define GEOMETRYGENERATOR_H

#include <windows.h>
#include <vector>

#include "EMath.h"

/// <summary>
/// 용책에 있던 클래스
/// Axis, Grid, Frustum까지 만들 수 있도록 개조함
/// 2021. 05. 정종영
/// </summary>
class GeometryGenerator
{
public:
	struct GenVertex
	{
		GenVertex() { ZeroMemory(this, sizeof(this)); }
		GenVertex(const EMath::FLOAT3& p, const EMath::FLOAT3& n, const EMath::FLOAT3& t, const EMath::FLOAT2& uv)
			: Position(p), Normal(n), TangentU(t), TexC(uv), Color(EMath::FLOAT4(0, 0, 0, 0)) {}
		GenVertex
		(float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v)
			: Position(px, py, pz), Normal(nx, ny, nz),
			TangentU(tx, ty, tz), TexC(u, v), Color(EMath::FLOAT4(0, 0, 0, 0)) {}

		EMath::FLOAT3 Position;
		EMath::FLOAT3 Normal;
		EMath::FLOAT3 TangentU;
		EMath::FLOAT2 TexC;
		EMath::FLOAT4 Color;
	};

	struct GeoData
	{
		std::vector<GenVertex> Vertices;
		std::vector<UINT> Indices;
	};

	///<summary>
	/// Creates a box centered at the origin with the given dimensions.
	///</summary>
	void CreateBox(float width, float height, float depth, GeoData& meshData);
	void CreateBox(float widthMin, float widthMax, float heightMin, float heightMax, float depthMin, float depthMax, GeoData& meshData);
	void CreateBox(EMath::Vector3 center, EMath::Vector3 extents, GeoData& meshData);
	///<summary>
	/// Creates a sphere centered at the origin with the given radius.  The
	/// slices and stacks parameters control the degree of tessellation.
	///</summary>
	void CreateSphere(float radius, UINT sliceCount, UINT stackCount, GeoData& meshData);

	///<summary>
	/// Creates a geosphere centered at the origin with the given radius.  The
	/// depth controls the level of tessellation.
	///</summary>
	void CreateGeosphere(float radius, UINT numSubdivisions, GeoData& meshData);

	///<summary>
	/// Creates a cylinder parallel to the y-axis, and centered about the origin.  
	/// The bottom and top radius can vary to form various cone shapes rather than true
	// cylinders.  The slices and stacks parameters control the degree of tessellation.
	///</summary>
	void CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, GeoData& meshData);

	void CreateSkull(GeoData& meshData);

	///<summary>
	/// Creates an mxn grid in the xz-plane with m rows and n columns, centered
	/// at the origin with the specified width and depth.
	///</summary>
	// 디버그용 그리드
	void CreateGrid(float width, float depth, size_t m, size_t n, GeoData& meshData);
	void CreateGrid(size_t mn, GeoData& meshData);

	// 디버그용 축을 생성한다.
	void CreateAxis(float length, GeoData& meshData);

	// 디버그용 프러스텀
	void CreateFrustum(EMath::Matrix view, EMath::Matrix proj, GeoData& meshData);

	///<summary>
	/// Creates a quad covering the screen in NDC coordinates.  This is useful for
	/// postprocessing effects.
	///</summary>
	void CreateFullscreenQuad(GeoData& meshData);

	// Skycube용 Cube 메쉬 생성
	void CreateCube(GeoData& meshData);

private:
	void Subdivide(GeoData& meshData);
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, GeoData& meshData);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, GeoData& meshData);
};

#endif // GEOMETRYGENERATOR_H