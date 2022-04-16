#pragma once

enum class RenderTargetOrder
{
	None,
	Albedo,			// Color
	Material,		// Material(Ambient, Diffuse, Specular, Reflection)
	PosW,			// Pos in World
	PosH,			// Pos in homogeneous (NDC)
	NormalW,		// Normal in World
	ShadowPosH,		// Tangent
	NormalDepth,	// NormalV, PosV.z
};