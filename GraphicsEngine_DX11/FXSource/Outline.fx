//=============================================================================
	// 물체의 외곽선을 그린다.
//=============================================================================

float4x4 gWorldInvTranspose;		// 노말
float4x4 gWorldViewProj;

cbuffer cbSkinned
{
	float4x4 gBoneTransforms[96];
};

Texture2D gNormalW;
Texture2D gFinal;
Texture2D gOutline;
Texture2D gDepthMap;

float gSize;		// 외곽선 두께

float mask[9] =
{ -1, -1, -1,
  -1,  8, -1,
  -1, -1, -1 }; // Laplacian Filter

float coord[3] = { -1, 0, +1 };
float divider = 1;
float MAP_CX = 1920 / 2;
float MAP_CY = 1080 / 2;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VertexIn_Basic
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexIn_NormalMap
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
	float3 TangentL : TANGENT;
};

struct VertexIn_Skinning
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
	float3 TangentL : TANGENT;
	float3 Weights : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 NormalW : NORMAL0;
};

struct VertexIn_RenderTarget
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut_RenderTarget
{
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
};

VertexOut VS_Basic(VertexIn_Basic vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	return vout;
}

VertexOut VS_NormalMap(VertexIn_NormalMap vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	return vout;
}

VertexOut VS_Skinning(VertexIn_Skinning vin)
{
	VertexOut vout;

	// 버텍스 Weight 배열을 초기화한다. 아니면 SV_POSITION 값에 대한 이상한 경고를 볼 것이다.
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; ++i)
	{
		// nonuniform 스케일링이 없다고 가정하므로, 노말값을 위한 역행렬의 전치행렬이 필요없다.
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}

	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	vout.NormalW = mul(normalL, (float3x3)gWorldInvTranspose);
	return vout;
}

VertexOut VS_Basic_OverSize(VertexIn_Basic vin)
{
	VertexOut vout;
	vin.PosL.xyz += vin.NormalL * gSize;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	return vout;
}

VertexOut VS_NormalMap_OverSize(VertexIn_NormalMap vin)
{
	VertexOut vout;
	vin.PosL.xyz += vin.NormalL * gSize;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	return vout;
}

VertexOut VS_Skinning_OverSize(VertexIn_Skinning vin)
{
	VertexOut vout;

	// 버텍스 Weight 배열을 초기화한다. 아니면 SV_POSITION 값에 대한 이상한 경고를 볼 것이다.
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);

	//vin.PosL.xyz += vin.NormalL * gSize;
	for (int i = 0; i < 4; ++i)
	{
		// nonuniform 스케일링이 없다고 가정하므로, 노말값을 위한 역행렬의 전치행렬이 필요없다.
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}
	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	vout.NormalW = mul(normalL, (float3x3)gWorldInvTranspose);
	return vout;
}

float4 PS_Normal(VertexOut pin) : SV_Target
{
	float4 pOut = float4(normalize(pin.NormalW).xyz, 1.0f);
	
	return pOut;
}

float4 PS_OverSize(VertexOut pin) : SV_Target
{
	float4 pOut;
	pOut = pin.PosH;

	return pOut;
}

VertexOut_RenderTarget VS_Outline(VertexIn_RenderTarget vin)
{
	VertexOut_RenderTarget vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS_Outline_Laplacian(VertexOut_RenderTarget pin) : SV_Target
{
	float4 Color = 0;
	float4 Ret = 0;
	float3 grayScale = float3(0.3, 0.59, 0.11);

	for (int i = 0; i < 9; i++)
		Color += mask[i] * (gNormalW.Sample(samLinear, pin.Tex + float2(coord[i % 3] / MAP_CX, coord[i / 3] / MAP_CY)));

	float gray = 1 - dot(Color, grayScale);


	if (gray < 1)
	{
		//Ret = float4(gray, gray, gray, 1);
		Ret = float4(gray, gray, gray, 1) / divider;
		Ret.xyz = 1 - Ret.xyz;
	}

	return Ret;
}

float4 PS_Mix_Laplacian(VertexOut_RenderTarget pin) : SV_Target
{
	float4 final = float4(gFinal.Sample(samLinear, pin.Tex).xyz, 1.0f);

	float4 outline = float4(gOutline.Sample(samLinear, pin.Tex).xyz, 1.0f);

	float depth = gDepthMap.Sample(samLinear, pin.Tex).x;

	//if (depth.x >= 1.0f && outline.z <= 1.0f)
		final.xyz += outline.x;

	return final;
}

float4 PS_Mix_OverSize(VertexOut_RenderTarget pin) : SV_Target
{
	// 원본 화면
	float4 final = float4(gFinal.Sample(samLinear, pin.Tex).xyz, 1.0f);
	// vertex + normal 한 것 가져오기
	float4 outline = float4(gOutline.Sample(samLinear, pin.Tex).xyz, 1.0f);
	// 뎁스 기록한 것 가져오기
	float depth = gDepthMap.Sample(samLinear, pin.Tex).x;

	// 서로 뎁스 비교
	if (depth.x >= 1.0f && outline.z <= 1.0f)
		final.xyz += outline.xyz;

	return float4(final.xyz, 1.0f);
}

technique11 Basic_Depth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Basic()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);
	}
}

technique11 NormalMap_Depth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_NormalMap()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);
	}
}

technique11 Skinning_Depth
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Skinning()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);
	}
}

technique11 Basic_Normal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Basic()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Normal()));
	}
}

technique11 NormalMap_Normal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_NormalMap()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Normal()));
	}
}

technique11 Skinning_Normal
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Skinning()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Normal()));
	}
}

technique11 Basic_OverSize
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Basic_OverSize()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_OverSize()));
	}
}

technique11 NormalMap_OverSize
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_NormalMap_OverSize()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_OverSize()));
	}
}

technique11 Skinning_OverSize
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Skinning_OverSize()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_OverSize()));
	}
}

technique11 Outline_Laplacian
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Outline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Outline_Laplacian()));
	}
}

technique11 Mix_Laplacian
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Outline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Mix_Laplacian()));
	}
}

technique11 Mix_OverSize
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Outline()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Mix_OverSize()));
	}
}
