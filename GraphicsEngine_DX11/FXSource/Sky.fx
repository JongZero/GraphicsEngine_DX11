//=============================================================================
// Sky.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Effect used to shade sky dome.
//=============================================================================

cbuffer cbPerFrame
{
	float4x4 gWorldViewProj;
	float4x4 gView;
	float4x4 gProj;
	float gRoughness;
};
 
// Nonnumeric values cannot be added to a cbuffer.
TextureCube gCubeMap;
Texture2D gTexture;

SamplerState samTriLinearSam
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn
{
	float3 PosL : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 PosL : TEXCOORD1;
	float2 Tex : TEXCOORD2;
};

// MRT
struct PSOut
{
	float4 Albedo : SV_Target0;		// Color
	float4 Material : SV_Target1;	// Material
	float4 PosW : SV_Target2;		// Pos in World
	float4 PosH : SV_Target3;		// Pos in homogeneous (NDC).xy, Depth(z)
	float4 NormalW : SV_Target4;	// NormalW
	float4 ShadowPosH : SV_Target5;	// Shadow Pos in H

	float4 ForBlur : SV_Target6;		// Blur(Color)
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.Tex = vin.Tex;
	
	// Use local vertex position as cubemap lookup vector.
	vout.PosL = vin.PosL;
	
	/*
	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj).xyww;
	*/
	
	// PBR, IBL
	float4x4 newView = gView;
	newView[3][0] = 0.0;
	newView[3][1] = 0.0;
	newView[3][2] = 0.0;
	
	float4 vPos = float4(vin.PosL.xyz, 1.0);
	vout.PosH = mul(vPos, newView);
	vout.PosH = mul(vout.PosH, gProj);
	
	vout.PosH = vout.PosH.xyzw;
	vout.PosH.z = vout.PosH.w * 0.9999;
	
	return vout;
}

float4 PS_Forward(VertexOut pin) : SV_Target
{
	float3 litColor = gCubeMap.Sample(samTriLinearSam, pin.PosL).xyz;
	
	/*
	litColor = litColor / (litColor + float3(1.0, 1.0, 1.0));
	litColor = pow(litColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
	*/	

	return float4(litColor, 1.0);
}

float4 PS_Deferred(VertexOut pin) : SV_Target
{
	PSOut pOut;
	float4 litColor = gCubeMap.Sample(samTriLinearSam, pin.PosL);

	pOut.Albedo = litColor;
	pOut.Material = float4(0, 0, 0, 0);
	pOut.PosW = float4(0, 0, 0, 1);
	pOut.PosH = float4(0.5, 0, 0, 1);
	pOut.NormalW = float4(0, 0, 0, 1);
	pOut.ShadowPosH = float4(1, 0, 0.0, 0);
	pOut.ForBlur = litColor;

	return litColor;
}

// RectToCubeMap
static const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
	float2 uv = float2(atan2(v.z, v.x), asin(v.y));
	uv *= invAtan;
	uv += 0.5;
	return uv;
}

float4 PS_RectToCubeMap(VertexOut pin) : SV_Target
{
	float2 uv = SampleSphericalMap(normalize(pin.PosL));
	float3 colour = gTexture.Sample(samTriLinearSam, uv).rgb;
		
	return float4(colour, 1.0);
}
// end RectToCubeMap

// Irradiance
static const float PI = 3.14159265359;

float4 PS_Irradiance(VertexOut pin) : SV_Target
{
	float3 normal = pin.PosL;
	float3 irradiance = float3(0.0, 0.0, 0.0);

	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = cross(up, normal);
	up = cross(normal, right);

	float sampleDelta = 0.025;
	float nrSamples = 0.0;
	for(float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
	{
		for(float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
		{
			// spherical to cartesian (in tangent space)
			float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// tangent space to world
			float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

			irradiance += gCubeMap.Sample(samTriLinearSam, sampleVec).rgb * cos(theta) * sin(theta);
			nrSamples++;
		}
	}
	irradiance = PI * irradiance * (1.0 / float(nrSamples));

	return float4(irradiance, 1.0);
}
// end Irradiance

// PreFilter
float RadicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}
// ----------------------------------------------------------------------------
float2 Hammersley(uint i, uint N)
{
	return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
	float a = roughness * roughness;

	float phi = 2.0 * PI * Xi.x;
	float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// from spherical coordinates to cartesian coordinates
	float3 H;
	H.x = cos(phi) * sinTheta;
	H.y = sin(phi) * sinTheta;
	H.z = cosTheta;

	// from tangent-space vector to world-space sample vector
	float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
	float3 tangent = normalize(cross(up, N));
	float3 bitangent = cross(N, tangent);

	float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
	return normalize(sampleVec);
}

float4 PS_PreFilter(VertexOut pin) : SV_Target
{
	float roughness = gRoughness;
	float3 N = normalize(pin.PosL);
	float3 R = N;
	float3 V = R;

	const uint SAMPLE_COUNT = 1024u;
	float totalWeight = 0.0;
	float3 prefilteredColor = float3(0.0, 0.0, 0.0);
	for(uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(dot(N, L), 0.0);
		if(NdotL > 0.0)
		{
			prefilteredColor += gCubeMap.Sample(samTriLinearSam, L).rgb * NdotL;
			totalWeight += NdotL;
		}
	}
	prefilteredColor = prefilteredColor / totalWeight;

	return float4(prefilteredColor, 1.0);
}
// end PreFilter

// IntegrateBRDF

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float a = roughness;
	float k = (a * a) / 2.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

float2 IntegrateBRDF(float NdotV, float roughness)
{
	float3 V;
	V.x = sqrt(1.0 - NdotV * NdotV);
	V.y = 0.0;
	V.z = NdotV;

	float A = 0.0;
	float B = 0.0;

	float3 N = float3(0.0, 0.0, 1.0);

	const uint SAMPLE_COUNT = 1024u;
	for(uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		float2 Xi = Hammersley(i, SAMPLE_COUNT);
		float3 H = ImportanceSampleGGX(Xi, N, roughness);
		float3 L = normalize(2.0 * dot(V, H) * H - V);

		float NdotL = max(L.z, 0.0);
		float NdotH = max(H.z, 0.0);
		float VdotH = max(dot(V, H), 0.0);

		if(NdotL > 0.0)
		{
			float G = GeometrySmith(N, V, L, roughness);
			float G_Vis = (G * VdotH) / (NdotH * NdotV);
			float Fc = pow(1.0 - VdotH, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	A /= float(SAMPLE_COUNT);
	B /= float(SAMPLE_COUNT);
	return float2(A, B);
}

float4 PS_IntegrateBRDF(VertexOut pin) : SV_Target
{
	float2 integratedBRDF = IntegrateBRDF(pin.Tex.x, pin.Tex.y);
	return float4(integratedBRDF, 0.0, 1.0);
}

// end IntegrateBRDF

RasterizerState NoCull
{
    CullMode = None;
};

DepthStencilState LessEqualDSS
{
	// Make sure the depth function is LESS_EQUAL and not just LESS.  
	// Otherwise, the normalized depth values at z = 1 (NDC) will 
	// fail the depth test if the depth buffer was cleared to 1.
    DepthFunc = LESS_EQUAL;
};

technique11 SkyTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS_Forward() ) );
        
        SetRasterizerState(NoCull);
        SetDepthStencilState(LessEqualDSS, 0);
    }
}

technique11 DeferredSky
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Deferred()));

		SetRasterizerState(NoCull);
		SetDepthStencilState(LessEqualDSS, 0);
	}
}

technique11 RectToCubeMapTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_RectToCubeMap()));
	}
}

technique11 IrradianceTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Irradiance()));
	}
}

technique11 PreFilterTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_PreFilter()));
	}
}

technique11 IntegrateBRDFTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_IntegrateBRDF()));
	}
}