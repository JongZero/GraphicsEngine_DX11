//***************************************************************************************
// GeometryGenerator.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "GeometryGenerator.h"
#include "Vertex.h"

#include <MathHelper.h>
#include <fstream>
#include "EColors.h"

void GeometryGenerator::CreateBox(float width, float height, float depth, GeoData& meshData)
{
	//
	// Create the vertices.
	//

	GenVertex v[24];

	float w2 = 0.5f * width;
	float h2 = 0.5f * height;
	float d2 = 0.5f * depth;

	// Fill in the front face vertex data.
	v[0] = GenVertex(-w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = GenVertex(-w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = GenVertex(+w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = GenVertex(+w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = GenVertex(-w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = GenVertex(+w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = GenVertex(+w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = GenVertex(-w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = GenVertex(-w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = GenVertex(-w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = GenVertex(+w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = GenVertex(+w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = GenVertex(-w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = GenVertex(+w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = GenVertex(+w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = GenVertex(-w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = GenVertex(-w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = GenVertex(-w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = GenVertex(-w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = GenVertex(-w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = GenVertex(+w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = GenVertex(+w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = GenVertex(+w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = GenVertex(+w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	UINT i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices.assign(&i[0], &i[36]);
}

void GeometryGenerator::CreateBox(float widthMin, float widthMax, float heightMin, float heightMax, float depthMin, float depthMax, GeoData& meshData)
{
	//
	// Create the vertices.
	//

	float width = 0.5f * (widthMax - widthMin);
	float height = 0.5f * (heightMax - heightMin);
	float depth = 0.5f * (depthMax - depthMin);

	GenVertex v[24];

	float w2 = 0.5f * (widthMax + widthMin);
	float h2 = 0.5f * (heightMax + heightMin);
	float d2 = 0.5f * (depthMax + depthMin);

	// Fill in the front face vertex data.
	v[0] = GenVertex(w2 - width, h2 - height, d2 - depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = GenVertex(w2 - width, h2 + height, d2 - depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = GenVertex(w2 + width, h2 + height, d2 - depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = GenVertex(w2 + width, h2 - height, d2 - depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = GenVertex(w2 - width, h2 - height, d2 + depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = GenVertex(w2 + width, h2 - height, d2 + depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = GenVertex(w2 + width, h2 + height, d2 + depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = GenVertex(w2 - width, h2 + height, d2 + depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = GenVertex(w2 - width, h2 + height, d2 - depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = GenVertex(w2 - width, h2 + height, d2 + depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = GenVertex(w2 + width, h2 + height, d2 + depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = GenVertex(w2 + width, h2 + height, d2 - depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = GenVertex(w2 - width, h2 - height, d2 - depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = GenVertex(w2 + width, h2 - height, d2 - depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = GenVertex(w2 + width, h2 - height, d2 + depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = GenVertex(w2 - width, h2 - height, d2 + depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = GenVertex(w2 - width, h2 - height, d2 + depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = GenVertex(w2 - width, h2 + height, d2 + depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = GenVertex(w2 - width, h2 + height, d2 - depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = GenVertex(w2 - width, h2 - height, d2 - depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = GenVertex(w2 + width, h2 - height, d2 - depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = GenVertex(w2 + width, h2 + height, d2 - depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = GenVertex(w2 + width, h2 + height, d2 + depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = GenVertex(w2 + width, h2 - height, d2 + depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	UINT i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices.assign(&i[0], &i[36]);
}

void GeometryGenerator::CreateBox(EMath::Vector3 center, EMath::Vector3 extents, GeoData& meshData)
{
	//
	// Create the vertices.
	//

	float width = extents.x;
	float height = extents.y;
	float depth = extents.z;

	GenVertex v[24];

	float w2 = center.x;
	float h2 = center.y;
	float d2 = center.z;

	// Fill in the front face vertex data.
	v[0] = GenVertex(w2 - width, h2 - height, d2 - depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[1] = GenVertex(w2 - width, h2 + height, d2 - depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[2] = GenVertex(w2 + width, h2 + height, d2 - depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[3] = GenVertex(w2 + width, h2 - height, d2 - depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the back face vertex data.
	v[4] = GenVertex(w2 - width, h2 - height, d2 + depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[5] = GenVertex(w2 + width, h2 - height, d2 + depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[6] = GenVertex(w2 + width, h2 + height, d2 + depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[7] = GenVertex(w2 - width, h2 + height, d2 + depth, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the top face vertex data.
	v[8] = GenVertex(w2 - width, h2 + height, d2 - depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[9] = GenVertex(w2 - width, h2 + height, d2 + depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[10] = GenVertex(w2 + width, h2 + height, d2 + depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	v[11] = GenVertex(w2 + width, h2 + height, d2 - depth, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

	// Fill in the bottom face vertex data.
	v[12] = GenVertex(w2 - width, h2 - height, d2 - depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	v[13] = GenVertex(w2 + width, h2 - height, d2 - depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
	v[14] = GenVertex(w2 + width, h2 - height, d2 + depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	v[15] = GenVertex(w2 - width, h2 - height, d2 + depth, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	// Fill in the left face vertex data.
	v[16] = GenVertex(w2 - width, h2 - height, d2 + depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f);
	v[17] = GenVertex(w2 - width, h2 + height, d2 + depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f);
	v[18] = GenVertex(w2 - width, h2 + height, d2 - depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f);
	v[19] = GenVertex(w2 - width, h2 - height, d2 - depth, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f);

	// Fill in the right face vertex data.
	v[20] = GenVertex(w2 + width, h2 - height, d2 - depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f);
	v[21] = GenVertex(w2 + width, h2 + height, d2 - depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
	v[22] = GenVertex(w2 + width, h2 + height, d2 + depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f);
	v[23] = GenVertex(w2 + width, h2 - height, d2 + depth, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);

	meshData.Vertices.assign(&v[0], &v[24]);

	//
	// Create the indices.
	//

	UINT i[36];

	// Fill in the front face index data
	i[0] = 0; i[1] = 1; i[2] = 2;
	i[3] = 0; i[4] = 2; i[5] = 3;

	// Fill in the back face index data
	i[6] = 4; i[7] = 5; i[8] = 6;
	i[9] = 4; i[10] = 6; i[11] = 7;

	// Fill in the top face index data
	i[12] = 8; i[13] = 9; i[14] = 10;
	i[15] = 8; i[16] = 10; i[17] = 11;

	// Fill in the bottom face index data
	i[18] = 12; i[19] = 13; i[20] = 14;
	i[21] = 12; i[22] = 14; i[23] = 15;

	// Fill in the left face index data
	i[24] = 16; i[25] = 17; i[26] = 18;
	i[27] = 16; i[28] = 18; i[29] = 19;

	// Fill in the right face index data
	i[30] = 20; i[31] = 21; i[32] = 22;
	i[33] = 20; i[34] = 22; i[35] = 23;

	meshData.Indices.assign(&i[0], &i[36]);
}

void GeometryGenerator::CreateSphere(float radius, UINT sliceCount, UINT stackCount, GeoData& meshData)
{
	meshData.Vertices.clear();
	meshData.Indices.clear();

	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.
	GenVertex topVertex(0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f);
	GenVertex bottomVertex(0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	meshData.Vertices.push_back(topVertex);

	float phiStep = EMath::PI / stackCount;
	float thetaStep = 2.0f * EMath::PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (UINT i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			GenVertex v;

			// spherical to cartesian
			v.Position.x = radius * sinf(phi) * cosf(theta);
			v.Position.y = radius * cosf(phi);
			v.Position.z = radius * sinf(phi) * sinf(theta);

			// Partial derivative of P with respect to theta
			v.TangentU.x = -radius * sinf(phi) * sinf(theta);
			v.TangentU.y = 0.0f;
			v.TangentU.z = +radius * sinf(phi) * cosf(theta);

			EMath::Vector3 T = v.TangentU;
			T.Normalize();
			v.TangentU = T;

			EMath::Vector3 p = v.Position;
			p.Normalize();
			v.Normal = p;

			v.TexC.x = theta / EMath::PI2;
			v.TexC.y = phi / EMath::PI;

			meshData.Vertices.push_back(v);
		}
	}

	meshData.Vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (UINT i = 1; i <= sliceCount; ++i)
	{
		meshData.Indices.push_back(0);
		meshData.Indices.push_back(i + 1);
		meshData.Indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	UINT baseIndex = 1;
	UINT ringVertexCount = sliceCount + 1;
	for (UINT i = 0; i < stackCount - 2; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.Indices.push_back(baseIndex + i * ringVertexCount + j);
			meshData.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);

			meshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j);
			meshData.Indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			meshData.Indices.push_back(baseIndex + (i + 1) * ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	UINT southPoleIndex = (UINT)meshData.Vertices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(southPoleIndex);
		meshData.Indices.push_back(baseIndex + i);
		meshData.Indices.push_back(baseIndex + i + 1);
	}
}

void GeometryGenerator::Subdivide(GeoData& meshData)
{
	// Save a copy of the input geometry.
	GeoData inputCopy = meshData;


	meshData.Vertices.resize(0);
	meshData.Indices.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	size_t numTris = inputCopy.Indices.size() / 3;
	for (size_t i = 0; i < numTris; ++i)
	{
		GenVertex v0 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 0]];
		GenVertex v1 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 1]];
		GenVertex v2 = inputCopy.Vertices[inputCopy.Indices[i * 3 + 2]];

		//
		// Generate the midpoints.
		//

		GenVertex m0, m1, m2;

		// For subdivision, we just care about the position component.  We derive the other
		// vertex components in CreateGeosphere.

		m0.Position = EMath::FLOAT3(
			0.5f * (v0.Position.x + v1.Position.x),
			0.5f * (v0.Position.y + v1.Position.y),
			0.5f * (v0.Position.z + v1.Position.z));

		m1.Position = EMath::FLOAT3(
			0.5f * (v1.Position.x + v2.Position.x),
			0.5f * (v1.Position.y + v2.Position.y),
			0.5f * (v1.Position.z + v2.Position.z));

		m2.Position = EMath::FLOAT3(
			0.5f * (v0.Position.x + v2.Position.x),
			0.5f * (v0.Position.y + v2.Position.y),
			0.5f * (v0.Position.z + v2.Position.z));

		//
		// Add new geometry.
		//

		meshData.Vertices.push_back(v0); // 0
		meshData.Vertices.push_back(v1); // 1
		meshData.Vertices.push_back(v2); // 2
		meshData.Vertices.push_back(m0); // 3
		meshData.Vertices.push_back(m1); // 4
		meshData.Vertices.push_back(m2); // 5

		meshData.Indices.push_back(i * 6 + 0);
		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 5);

		meshData.Indices.push_back(i * 6 + 5);
		meshData.Indices.push_back(i * 6 + 4);
		meshData.Indices.push_back(i * 6 + 2);

		meshData.Indices.push_back(i * 6 + 3);
		meshData.Indices.push_back(i * 6 + 1);
		meshData.Indices.push_back(i * 6 + 4);
	}
}

void GeometryGenerator::CreateGeosphere(float radius, UINT numSubdivisions, GeoData& meshData)
{
	// Put a cap on the number of subdivisions.
	numSubdivisions = MathHelper::Min(numSubdivisions, 5u);

	// Approximate a sphere by tessellating an icosahedron.

	const float X = 0.525731f;
	const float Z = 0.850651f;

	EMath::FLOAT3 pos[12] =
	{
		EMath::FLOAT3(-X, 0.0f, Z),  EMath::FLOAT3(X, 0.0f, Z),
		EMath::FLOAT3(-X, 0.0f, -Z), EMath::FLOAT3(X, 0.0f, -Z),
		EMath::FLOAT3(0.0f, Z, X),   EMath::FLOAT3(0.0f, Z, -X),
		EMath::FLOAT3(0.0f, -Z, X),  EMath::FLOAT3(0.0f, -Z, -X),
		EMath::FLOAT3(Z, X, 0.0f),   EMath::FLOAT3(-Z, X, 0.0f),
		EMath::FLOAT3(Z, -X, 0.0f),  EMath::FLOAT3(-Z, -X, 0.0f)
	};

	DWORD k[60] =
	{
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};
	
	meshData.Vertices.resize(12);
	meshData.Indices.resize(60);

	for (UINT i = 0; i < 12; ++i)
		meshData.Vertices[i].Position = pos[i];

	for (UINT i = 0; i < 60; ++i)
		meshData.Indices[i] = k[i];

	for (UINT i = 0; i < numSubdivisions; ++i)
		Subdivide(meshData);

	// Project vertices onto sphere and scale.
	for (UINT i = 0; i < meshData.Vertices.size(); ++i)
	{
		// Project onto unit sphere.
		EMath::Vector3 n = meshData.Vertices[i].Position;
		n.Normalize();

		// Project onto sphere.
		EMath::Vector3 p = radius * n;

		meshData.Vertices[i].Position = p;
		meshData.Vertices[i].Normal = n;

		// Derive texture coordinates from spherical coordinates.
		float theta = MathHelper::AngleFromXY(
			meshData.Vertices[i].Position.x,
			meshData.Vertices[i].Position.z);

		float phi = acosf(meshData.Vertices[i].Position.y / radius);

		meshData.Vertices[i].TexC.x = theta / EMath::PI2;
		meshData.Vertices[i].TexC.y = phi / EMath::PI;

		// Partial derivative of P with respect to theta
		meshData.Vertices[i].TangentU.x = -radius * sinf(phi) * sinf(theta);
		meshData.Vertices[i].TangentU.y = 0.0f;
		meshData.Vertices[i].TangentU.z = +radius * sinf(phi) * cosf(theta);

		EMath::Vector3 T = meshData.Vertices[i].TangentU;
		T.Normalize();
		meshData.Vertices[i].TangentU = T;
	}
}

void GeometryGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, UINT sliceCount, UINT stackCount, GeoData& meshData)
{
	meshData.Vertices.clear();
	meshData.Indices.clear();

	//
	// Build Stacks.
	// 

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	UINT ringCount = stackCount + 1;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for (UINT i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// vertices of ring
		float dTheta = 2.0f * EMath::PI / sliceCount;
		for (UINT j = 0; j <= sliceCount; ++j)
		{
			GenVertex vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.Position = EMath::FLOAT3(r * c, y, r * s);

			vertex.TexC.x = (float)j / sliceCount;
			vertex.TexC.y = 1.0f - (float)i / stackCount;

			// Cylinder can be parameterized as follows, where we introduce v
			// parameter that goes in the same direction as the v tex-coord
			// so that the bitangent goes in the same direction as the v tex-coord.
			//   Let r0 be the bottom radius and let r1 be the top radius.
			//   y(v) = h - hv for v in [0,1].
			//   r(v) = r1 + (r0-r1)v
			//
			//   x(t, v) = r(v)*cos(t)
			//   y(t, v) = h - hv
			//   z(t, v) = r(v)*sin(t)
			// 
			//  dx/dt = -r(v)*sin(t)
			//  dy/dt = 0
			//  dz/dt = +r(v)*cos(t)
			//
			//  dx/dv = (r0-r1)*cos(t)
			//  dy/dv = -h
			//  dz/dv = (r0-r1)*sin(t)

			// This is unit length.
			vertex.TangentU = EMath::FLOAT3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			EMath::FLOAT3 bitangent(dr * c, -height, dr * s);

			EMath::Vector3 T = vertex.TangentU;
			EMath::Vector3 B = bitangent;
			EMath::Vector3 N = T.Cross(B);
			N.Normalize();

			vertex.Normal = N;

			meshData.Vertices.push_back(vertex);
		}
	}

	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	UINT ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.
	for (UINT i = 0; i < stackCount; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			meshData.Indices.push_back(i * ringVertexCount + j);
			meshData.Indices.push_back((i + 1) * ringVertexCount + j);
			meshData.Indices.push_back((i + 1) * ringVertexCount + j + 1);

			meshData.Indices.push_back(i * ringVertexCount + j);
			meshData.Indices.push_back((i + 1) * ringVertexCount + j + 1);
			meshData.Indices.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, meshData);
}

void GeometryGenerator::CreateSkull(GeoData& meshData)
{
	std::ifstream fin("../Data/Models/skull.txt");

	if (!fin)
	{
		MessageBox(0, L"Models/skull.txt not found.", 0, 0);
		return;
	}

	UINT vcount = 0;
	UINT tcount = 0;
	UINT indexCount = 0;
	std::string ignore;

	fin >> ignore >> vcount;
	fin >> ignore >> tcount;
	fin >> ignore >> ignore >> ignore >> ignore;

	meshData.Vertices.clear();
	meshData.Indices.clear();

	meshData.Vertices.resize(vcount);

	std::vector<Vertex::PosNormalTex> vertices(vcount);
	for (UINT i = 0; i < vcount; ++i)
	{
		fin >> vertices[i].Pos.x >> vertices[i].Pos.y >> vertices[i].Pos.z;
		fin >> vertices[i].Normal.x >> vertices[i].Normal.y >> vertices[i].Normal.z;
	}

	fin >> ignore;
	fin >> ignore;
	fin >> ignore;

	indexCount = 3 * tcount;
	std::vector<UINT> indices(indexCount);
	for (size_t i = 0; i < tcount; ++i)
	{
		fin >> indices[i * 3 + 0] >> indices[i * 3 + 1] >> indices[i * 3 + 2];
	}

	fin.close();

	meshData.Indices.assign(indices.begin(), indices.end());

	for (UINT i = 0; i < vcount; ++i)
	{
		meshData.Vertices[i].Position = vertices[i].Pos;
		meshData.Vertices[i].Normal = vertices[i].Normal;
		meshData.Vertices[i].TexC = EMath::FLOAT2(0, 0);
	}
}

void GeometryGenerator::BuildCylinderTopCap(float bottomRadius, float topRadius, float height,
	UINT sliceCount, UINT stackCount, GeoData& meshData)
{
	UINT baseIndex = (UINT)meshData.Vertices.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * EMath::PI / sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(GenVertex(x, y, z, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(GenVertex(0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Index of center vertex.
	UINT centerIndex = (UINT)meshData.Vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i + 1);
		meshData.Indices.push_back(baseIndex + i);
	}
}

void GeometryGenerator::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
	UINT sliceCount, UINT stackCount, GeoData& meshData)
{
	// 
	// Build bottom cap.
	//

	UINT baseIndex = (UINT)meshData.Vertices.size();
	float y = -0.5f * height;

	// vertices of ring
	float dTheta = 2.0f * EMath::PI / sliceCount;
	for (UINT i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		meshData.Vertices.push_back(GenVertex(x, y, z, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, u, v));
	}

	// Cap center vertex.
	meshData.Vertices.push_back(GenVertex(0.0f, y, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 0.5f));

	// Cache the index of center vertex.
	UINT centerIndex = (UINT)meshData.Vertices.size() - 1;

	for (UINT i = 0; i < sliceCount; ++i)
	{
		meshData.Indices.push_back(centerIndex);
		meshData.Indices.push_back(baseIndex + i);
		meshData.Indices.push_back(baseIndex + i + 1);
	}
}

void GeometryGenerator::CreateGrid(float width, float depth, size_t m, size_t n, GeoData& meshData)
{
	size_t vertexCount = m * n;
	size_t faceCount = (m - 1) * (n - 1) * 2;

	//
	// Create the vertices.
	//

	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;

	float dx = width / (n - 1);
	float dz = depth / (m - 1);

	float du = 1.0f / (n - 1);
	float dv = 1.0f / (m - 1);

	meshData.Vertices.resize(vertexCount);
	for (size_t i = 0; i < m; ++i)
	{
		float z = halfDepth - i * dz;
		for (size_t j = 0; j < n; ++j)
		{
			float x = -halfWidth + j * dx;

			meshData.Vertices[i * n + j].Position = EMath::FLOAT3(x, 0.0f, z);
			meshData.Vertices[i * n + j].Normal = EMath::FLOAT3(0.0f, 1.0f, 0.0f);
			meshData.Vertices[i * n + j].TangentU = EMath::FLOAT3(1.0f, 0.0f, 0.0f);

			// Stretch texture over grid.
			meshData.Vertices[i * n + j].TexC.x = j * du;
			meshData.Vertices[i * n + j].TexC.y = i * dv;
		}
	}

	//
	// Create the indices.
	//

	meshData.Indices.resize(faceCount * 3); // 3 indices per face

	// Iterate over each quad and compute indices.
	size_t k = 0;
	for (size_t i = 0; i < m - 1; ++i)
	{
		for (size_t j = 0; j < n - 1; ++j)
		{
			meshData.Indices[k] = i * n + j;
			meshData.Indices[k + 1] = i * n + j + 1;
			meshData.Indices[k + 2] = (i + 1) * n + j;

			meshData.Indices[k + 3] = (i + 1) * n + j;
			meshData.Indices[k + 4] = i * n + j + 1;
			meshData.Indices[k + 5] = (i + 1) * n + j + 1;

			k += 6; // next quad
		}
	}
}

void GeometryGenerator::CreateGrid(size_t mn, GeoData& meshData)
{
	size_t vertexCount = mn * mn;
	size_t faceCount = 4 * mn;

	meshData.Vertices.resize(vertexCount);
	for (UINT i = 0; i < vertexCount; i++)
	{
		meshData.Vertices[i].Position = EMath::FLOAT3((float)(i % mn) - mn / 2, 0.0f, (float)(i / mn) - mn / 2);
		meshData.Vertices[i].Color = EMath::FLOAT4(EColors::DarkGray);
	}

	meshData.Indices.resize(faceCount);
	for (size_t i = 0; i < mn; i++)
	{
		meshData.Indices[i * 2] = i;
		meshData.Indices[i * 2 + 1] = i + (vertexCount - mn);
	}

	for (size_t i = 0; i < mn; i++)
	{
		meshData.Indices[(mn * 2) + (i * 2)] = i * mn;
		meshData.Indices[(mn * 2) + (i * 2) + 1] = i * mn + (mn - 1);
	}
}

void GeometryGenerator::CreateAxis(float length, GeoData& meshData)
{
	UINT vertexCount = 6;
	UINT faceCount = 6;

	meshData.Vertices.resize(vertexCount);

	// X 축
	meshData.Vertices[0].Position = EMath::FLOAT3(0.0f, 0.0f, 0.0f);
	meshData.Vertices[0].Color = EMath::FLOAT4(EColors::Red);
	meshData.Vertices[1].Position = EMath::FLOAT3(length, 0.0f, 0.0f);
	meshData.Vertices[1].Color = EMath::FLOAT4(EColors::Red);

	// Y 축
	meshData.Vertices[2].Position = EMath::FLOAT3(0.0f, 0.0f, 0.0f);
	meshData.Vertices[2].Color = EMath::FLOAT4(EColors::Green);
	meshData.Vertices[3].Position = EMath::FLOAT3(0.0f, length, 0.0f);
	meshData.Vertices[3].Color = EMath::FLOAT4(EColors::Green);

	// Z 축
	meshData.Vertices[4].Position = EMath::FLOAT3(0.0f, 0.0f, 0.0f);
	meshData.Vertices[4].Color = EMath::FLOAT4(EColors::Blue);
	meshData.Vertices[5].Position = EMath::FLOAT3(0.0f, 0.0f, length);
	meshData.Vertices[5].Color = EMath::FLOAT4(EColors::Blue);

	meshData.Indices.resize(faceCount);

	for (UINT i = 0; i < meshData.Indices.size(); i++)
	{
		meshData.Indices[i] = i;
	}
}

void GeometryGenerator::CreateFrustum(EMath::Matrix view, EMath::Matrix proj, GeoData& meshData)
{
	UINT vertexCount = 5;
	UINT faceCount = 18;

	/*
	Vector4 HomogenousPoints[8] =
	{
		{ 1.0f, -1.0f, 1.0f, 1.0f },	// right bottom(at far plane)

		{ -1.0f, -1.0f, 1.0f, 1.0f },	// left	bottom

		{ 1.0f, 1.0f, 1.0f, 1.0f },		// right top

		{ -1.0f, 1.0f, 1.0f, 1.0f },	// left top

		{ 1.0f, -1.0f, 0.0f, 1.0f },	// right bottom(at near plane)

		{ -1.0f, -1.0f, 0.0f, 1.0f },	// left	bottom

		{ 1.0f, 1.0f, 0.0f, 1.0f },		// right top

		{ -1.0f, 1.0f, 0.0f, 1.0f }		// left top
	};
	*/

	EMath::Vector4 HomogenousPoints[5] =
	{
		{ 1.0f, -1.0f, 1.0f, 1.0f },	// right bottom(at far plane)

		{ -1.0f, -1.0f, 1.0f, 1.0f },	// left	bottom

		{ 1.0f, 1.0f, 1.0f, 1.0f },		// right top 

		{ -1.0f, 1.0f, 1.0f, 1.0f },	// left top

		{ 0.0f, 0.0f, 0.0f, 1.0f }, 	// near
	};

	EMath::Matrix viewProj = view * proj;
	EMath::Matrix invViewProj = viewProj.Invert();

	// Compute the frustum corners in world space.
	EMath::Vector4 Points[5];

	for (UINT i = 0; i < 5; i++)
	{
		// Transform point.
		Points[i] = EMath::Vector4::Transform(HomogenousPoints[i], invViewProj);
	}

	meshData.Vertices.resize(5);

	for (UINT i = 0; i < meshData.Vertices.size(); i++)
	{
		EMath::Vector4 float4Point = Points[i];

		meshData.Vertices[i].Position.x = float4Point.x / float4Point.w;
		meshData.Vertices[i].Position.y = float4Point.y / float4Point.w;
		meshData.Vertices[i].Position.z = float4Point.z / float4Point.w;
	}

	// 인덱스 버퍼를 생성한다.
	/*
	UINT indices[] = {
		// backface
		0, 2, 1,
		1, 2, 3,

		// right
		6, 2, 0,
		6, 0, 4,

		// front
		7, 6, 4,
		5, 7, 4,

		// left
		7, 1, 3,
		5, 1, 7,

		// top
		3, 2, 6,
		7, 3, 6,

		// bottom
		0, 1, 4,
		4, 1, 5
	};
	*/

	UINT indices[] = {
		// backface
		0, 2, 1,
		1, 2, 3,

		// right
		4, 2, 0,

		// left
		1, 3, 4,

		// top
		3, 2, 4,

		// bottom
		0, 1, 4,
	};

	meshData.Indices.resize(faceCount);

	for (UINT i = 0; i < meshData.Indices.size(); i++)
	{
		meshData.Indices[i] = indices[i];
	}
}

void GeometryGenerator::CreateFullscreenQuad(GeoData& meshData)
{
	meshData.Vertices.resize(4);
	meshData.Indices.resize(6);

	// Position coordinates specified in NDC space.
	// Store far plane frustum corner indices in Normal.x slot.
	meshData.Vertices[0] = GenVertex(
		-1.0f, -1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f);

	meshData.Vertices[1] = GenVertex(
		-1.0f, +1.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		0.0f, 0.0f);

	meshData.Vertices[2] = GenVertex(
		+1.0f, +1.0f, 0.0f,
		2.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 0.0f);

	meshData.Vertices[3] = GenVertex(
		+1.0f, -1.0f, 0.0f,
		3.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f);

	meshData.Indices[0] = 0;
	meshData.Indices[1] = 1;
	meshData.Indices[2] = 2;

	meshData.Indices[3] = 0;
	meshData.Indices[4] = 2;
	meshData.Indices[5] = 3;
}

void GeometryGenerator::CreateCube(GeoData& meshData)
{
	meshData.Vertices.resize(24);
	meshData.Indices.resize(36);

	const float length = 1.0f;
	const float width = 1.0f;
	const float height = 1.0f;

	EMath::FLOAT3 p0 = EMath::FLOAT3(-length * .5f, -width * .5f, height * .5f);
	EMath::FLOAT3 p1 = EMath::FLOAT3(length * .5f, -width * .5f, height * .5f);
	EMath::FLOAT3 p2 = EMath::FLOAT3(length * .5f, -width * .5f, -height * .5f);
	EMath::FLOAT3 p3 = EMath::FLOAT3(-length * .5f, -width * .5f, -height * .5f);

	EMath::FLOAT3 p4 = EMath::FLOAT3(-length * .5f, width * .5f, height * .5f);
	EMath::FLOAT3 p5 = EMath::FLOAT3(length * .5f, width * .5f, height * .5f);
	EMath::FLOAT3 p6 = EMath::FLOAT3(length * .5f, width * .5f, -height * .5f);
	EMath::FLOAT3 p7 = EMath::FLOAT3(-length * .5f, width * .5f, -height * .5f);

	meshData.Vertices[0].Position = p0;
	meshData.Vertices[1].Position = p1;
	meshData.Vertices[2].Position = p2;
	meshData.Vertices[3].Position = p3;

	meshData.Vertices[4].Position = p7;
	meshData.Vertices[5].Position = p4;
	meshData.Vertices[6].Position = p0;
	meshData.Vertices[7].Position = p3;

	meshData.Vertices[8].Position = p4;
	meshData.Vertices[9].Position = p5;
	meshData.Vertices[10].Position = p1;
	meshData.Vertices[11].Position = p0;

	meshData.Vertices[12].Position = p6;
	meshData.Vertices[13].Position = p7;
	meshData.Vertices[14].Position = p3;
	meshData.Vertices[15].Position = p2;

	meshData.Vertices[16].Position = p5;
	meshData.Vertices[17].Position = p6;
	meshData.Vertices[18].Position = p2;
	meshData.Vertices[19].Position = p1;

	meshData.Vertices[20].Position = p7;
	meshData.Vertices[21].Position = p6;
	meshData.Vertices[22].Position = p5;
	meshData.Vertices[23].Position = p4;

	EMath::FLOAT2 _00 = EMath::FLOAT2(0.0f, 0.0f);
	EMath::FLOAT2 _10 = EMath::FLOAT2(1.0f, 0.0f);
	EMath::FLOAT2 _01 = EMath::FLOAT2(0.0f, 1.0f);
	EMath::FLOAT2 _11 = EMath::FLOAT2(1.0f, 1.0f);

	for (size_t i = 0; i < 6; ++i)
	{
		meshData.Vertices[i * 4].TexC = _11;
		meshData.Vertices[i * 4 + 1].TexC = _01;
		meshData.Vertices[i * 4 + 2].TexC = _00;
		meshData.Vertices[i * 4 + 3].TexC = _10;
	}

	int index = 0;
	for (int i = 0; i < 6; ++i)
	{
		meshData.Indices[index++] = i * 4;
		meshData.Indices[index++] = i * 4 + 1;
		meshData.Indices[index++] = i * 4 + 3;

		meshData.Indices[index++] = i * 4 + 1;
		meshData.Indices[index++] = i * 4 + 2;
		meshData.Indices[index++] = i * 4 + 3;
	}
}
