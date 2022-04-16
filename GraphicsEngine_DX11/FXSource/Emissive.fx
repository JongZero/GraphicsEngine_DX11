//=============================================================================
	// Emissive Map�� Ȱ���Ͽ� �߱�ȿ���� ����.
//=============================================================================

cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
};

cbuffer cbSkinned
{
	float4x4 gBoneTransforms[96];
};

Texture2D gEmissiveMap;
Texture2D gFinal;
Texture2D gEmissiveBluredOutput;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
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
	float2 Tex  : TEXCOORD;
};

VertexOut VS_Basic(VertexIn_Basic vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = vin.Tex;
	return vout;
}

VertexOut VS_NormalMap(VertexIn_NormalMap vin)
{
	VertexOut vout;
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = vin.Tex;
	return vout;
}

VertexOut VS_Skinning(VertexIn_Skinning vin)
{
	VertexOut vout;

	// ���ؽ� Weight �迭�� �ʱ�ȭ�Ѵ�. �ƴϸ� SV_POSITION ���� ���� �̻��� ��� �� ���̴�.
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL = float3(0.0f, 0.0f, 0.0f);

	for (int i = 0; i < 4; ++i)
	{
		// nonuniform �����ϸ��� ���ٰ� �����ϹǷ�, �븻���� ���� ������� ��ġ����� �ʿ����.
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
	}

	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	vout.Tex = vin.Tex;
	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	float4 pOut = float4(gEmissiveMap.Sample(samLinear, pin.Tex).xyz, 1.0f);
	pOut = pow(pOut, 2.2f);
	return pOut;
}

float4 PS_Mix(VertexOut pin) : SV_Target
{
	float4 final = float4(gFinal.Sample(samLinear, pin.Tex).xyz, 1.0f);
	float4 emissiveOutput = float4(gEmissiveBluredOutput.Sample(samLinear, pin.Tex).xyz, 1.0f);

	final.xyz += emissiveOutput.xyz;

	return final;
}

technique11 Basic
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Basic()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 NormalMap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_NormalMap()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 Skinning
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Skinning()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

technique11 Mix
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Basic()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS_Mix()));
	}
}
