// Shadow Map

cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
};

// Nonnumeric values cannot be added to a cbuffer.
// ���ġ ������ cbuffer�� �߰��� �� ����.
Texture2D gDiffuseMap;

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

struct VertexIn_NormalMap : VertexIn_Basic
{
	float3 TangentL : TANGENT;
};

struct VertexIn_Skinning : VertexIn_NormalMap
{
	float3 Weights : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 Tex     : TEXCOORD;
};

VertexOut VS_Basic(VertexIn_Basic vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}

VertexOut VS_NormalMap(VertexIn_NormalMap vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}

VertexOut VS_Skinning(VertexIn_Skinning vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}
 
// �� �ȼ� ���̴��� ���� �� ��� ���� ������ ���� ���ϱ������� ���δ�.
// ���� �н��� ��� �ؽ�ó�� ������ �ʿ䰡 ����
// ���ϱ����� ���ؼ��� NULL �ȼ� ���̴��� ����ص� �ȴ�.
void PS(VertexOut pin)
{
	float4 diffuse = gDiffuseMap.Sample(samLinear, pin.Tex);

	// ������ �ȼ����� �׸��� �ʿ� ��ϵ��� �ʰ� �Ѵ�.
	clip(diffuse.a - 0.15f);
}

RasterizerState Depth
{
	/* MSDN */
	// ���� ��� ���ձ� �ܰ迡 ���� ���� ������ ������ UNORM�̰ų�
	// ��� ���ձ� �ܰ迡 �ƹ��� ���� ���۵� ���� ���� �ʴٸ�,
	// ����ġ�� ������ ���� ���ȴ�.

	// Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
	
	// ���⼭ r�� ���� ���� ������ float32�� ��ȯ���� �� ǥ�� ������,
	// 0���� ū �ּڰ��̴�.
	/* MSDN �� */

	// 24��Ʈ ���� ������ ��� r = 1 / 2^24�̴�.
	// �� : DepthBias = 100000 ==> ���� DepthBias = 100000/2^24 = .006

	// �� ������ ����� Ư���� ũ�� �����ϹǷ�, ������ ��鿡
	// �� �´� ������ ���迡 ���ļ� ���ؾ� �� ���̴�.
	
	DepthBias = 100000;
	DepthBiasClamp = 0.0f;
	SlopeScaledDepthBias = 1.0f;
};

technique11 BuildShadowMap_Basic
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Basic()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 BuildShadowMap_NormalMap
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_NormalMap()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 BuildShadowMap_Skinning
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Skinning()));
		SetGeometryShader(NULL);
		SetPixelShader(NULL);

		SetRasterizerState(Depth);
	}
}

technique11 BuildShadowMapAlphaClip
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS_Basic()));
		SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));
    }
}