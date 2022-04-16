// Shadow Map

cbuffer cbPerObject
{
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
};

// Nonnumeric values cannot be added to a cbuffer.
// 비수치 값들은 cbuffer에 추가할 수 없다.
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
 
// 이 픽셀 셰이더는 알파 값 기반 투명 패턴을 가진 기하구조에만 쓰인다.
// 깊이 패스의 경우 텍스처를 추출할 필요가 없는
// 기하구조에 대해서는 NULL 픽셀 셰이더를 사용해도 된다.
void PS(VertexOut pin)
{
	float4 diffuse = gDiffuseMap.Sample(samLinear, pin.Tex);

	// 투명한 픽셀들은 그림자 맵에 기록되지 않게 한다.
	clip(diffuse.a - 0.15f);
}

RasterizerState Depth
{
	/* MSDN */
	// 현재 출력 병합기 단계에 묶인 깊이 버퍼의 형식이 UNORM이거나
	// 출력 병합기 단계에 아무런 깊이 버퍼도 묶여 있지 않다면,
	// 편향치는 다음과 같이 계산된다.

	// Bias = (float)DepthBias * r + SlopeScaledDepthBias * MaxDepthSlope;
	
	// 여기서 r은 깊이 버퍼 형식을 float32로 변환했을 때 표현 가능한,
	// 0보다 큰 최솟값이다.
	/* MSDN 끝 */

	// 24비트 깊이 버퍼의 경우 r = 1 / 2^24이다.
	// 예 : DepthBias = 100000 ==> 실제 DepthBias = 100000/2^24 = .006

	// 이 값들은 장면의 특성에 크게 의존하므로, 독자의 장면에
	// 잘 맞는 값들은 실험에 거쳐서 구해야 할 것이다.
	
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