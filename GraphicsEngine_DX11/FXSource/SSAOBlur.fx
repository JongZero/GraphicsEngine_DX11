//=============================================================================
// SsaoBlur.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs a bilateral edge preserving blur of the ambient map.  We use 
// a pixel shader instead of compute shader to avoid the switch from 
// compute mode to rendering mode.  The texture cache makes up for some of the
// loss of not having shared memory.  The ambient map uses 16-bit texture
// format, which is small, so we should be able to fit a lot of texels
// in the cache.
// SSAO 맵에 대해 양방향 가장자리 보존 흐리기를 수행한다.
// 계산 모드를 렌더링 모드로 전환하지 않기 위해 계산(컴퓨트) 셰이더 대신 픽셀 셰이더를 사용한다.
// 계산(컴퓨트) 셰이더 구현과는 달리 공유 메모리를 사용할 수 없으므로, 텍스쳐를 일종의 캐시로 사용한다.
// SSAO 맵은 16비트 텍스쳐 형식을 사용하므로 크기가 작다. 따라서 캐시에 많은 수의 텍셀을 담을 수 있다.
//=============================================================================

cbuffer cbPerFrame
{
	float gTexelWidth;
	float gTexelHeight;
};

cbuffer cbSettings
{
	float gWeights[11] = 
	{
		0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f, 0.05f
	};
};

cbuffer cbFixed
{
	static const int gBlurRadius = 5;
};
 
// Nonnumeric values cannot be added to a cbuffer.
// 비수치 값들은 상수 버퍼에 추가할 수 없다.
Texture2D gNormalDepthRT;
Texture2D gDepthRT;
Texture2D gInputImage;
 
SamplerState samNormalDepth
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

SamplerState samInputImage
{
	Filter = MIN_MAG_LINEAR_MIP_POINT;

	AddressU  = CLAMP;
    AddressV  = CLAMP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
    float4 PosH  : SV_POSITION;
	float2 Tex   : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Already in NDC space.
	// 이미 NDC 공간에 있다.
	vout.PosH = float4(vin.PosL, 1.0f);

	// Pass onto pixel shader.
	// 텍스쳐 좌표는 그대로 픽셀 셰이더에 넘겨준다.
	vout.Tex = vin.Tex;
	
    return vout;
}


float4 PS(VertexOut pin, uniform bool gHorizontalBlur) : SV_Target
{
	float2 texOffset;
	if(gHorizontalBlur)
	{
		texOffset = float2(gTexelWidth, 0.0f);
	}
	else
	{
		texOffset = float2(0.0f, gTexelHeight);
	}

	// The center value always contributes to the sum.
	// 필터 핵 중앙의 표본은 항상 총합에 기여한다.
	float4 color      = gWeights[5]*gInputImage.SampleLevel(samInputImage, pin.Tex, 0.0);
	float totalWeight = gWeights[5];
	 
	float4 centerNormalDepth = float4(float3(gNormalDepthRT.SampleLevel(samNormalDepth, pin.Tex, 0.0f).xyz), 
				gDepthRT.SampleLevel(samNormalDepth, pin.Tex, 0.0f).z);

	for(float i = -gBlurRadius; i <=gBlurRadius; ++i)
	{
		// We already added in the center weight.
		// 중앙 가중치는 이미 합산했다.
		if( i == 0 )
			continue;

		float2 tex = pin.Tex + i*texOffset;

		float4 neighborNormalDepth = float4(float3(gNormalDepthRT.SampleLevel(samNormalDepth, tex, 0.0f).xyz),
			gDepthRT.SampleLevel(samNormalDepth, tex, 0.0f).z);

		//
		// If the center value and neighbor values differ too much (either in 
		// normal or depth), then we assume we are sampling across a discontinuity.
		// We discard such samples from the blur.
		// 중앙의 값과 그 이웃 값의 차이가 너무 크면(법선이든, 깊이이든)
		// 필터 핵이 불연속성에(즉 가장자리에) 걸쳐 있는 것으로 간주해서,
		// 해당 표본들을 흐리기에서 제외시킨다.
		//
	
		if( dot(neighborNormalDepth.xyz, centerNormalDepth.xyz) >= 0.8f &&
		    abs(neighborNormalDepth.a - centerNormalDepth.a) <= 0.2f )
		{
			float weight = gWeights[i+gBlurRadius];

			// Add neighbor pixel to blur.
			// 흐릴 이웃 픽셀들을 추가한다.
			color += weight*gInputImage.SampleLevel(
				samInputImage, tex, 0.0);
		
			totalWeight += weight;
		}
	}

	// Compensate for discarded samples by making total weights sum to 1.
	// 계산에서 제외된 표본이 있을 수 있으므로,
	// 실제로 적용된 가중치들의 합으로 나누어 준다.
	return color / totalWeight;
}

technique11 HorzBlur
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(true) ) );
    }
}

technique11 VertBlur
{
    pass P0
    {
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(false) ) );
    }
}
 