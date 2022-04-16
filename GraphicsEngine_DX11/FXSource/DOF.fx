/********************************************/
				// DOF //
		// DirectX Tutorial 참고함 // 
/********************************************/

Texture2D gScreenRT;
Texture2D gBlurScreenRT;
Texture2D gDepthRT;

SamplerState samLinear
{
	//Filter = ANISOTROPIC;
	//MaxAnisotropy = 4;

	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

cbuffer cbPerFrame
{
	float gDepthStart;
	float gDepthRange;
	float gFarPlane;
	float gNearPlane;
};

float2 TexSize(Texture2D tex)
{
	uint texWidth, texHeight;
	tex.GetDimensions(texWidth, texHeight);
	return float2(texWidth, texHeight);
}

struct VertexIn
{
	float3 Pos    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 Pos    : SV_POSITION;
	float2 Tex     : TEXCOORD0;
};

float DepthBufferConvertToLinear(float depth)
{
	float a = 1.0 / (gNearPlane - gFarPlane);
	return (gNearPlane * gFarPlane * a) / (depth + gFarPlane * a);
}

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	vout.Pos = float4(vin.Pos.xy, 1.0f, 1.0f);
	vout.Tex = vin.Tex;

	return vout;
}
 
float4 PS(VertexOut pin) : SV_Target
{
	float4 color = gScreenRT.Sample(samLinear, pin.Tex);
	float4 blurColor = gBlurScreenRT.Sample(samLinear, pin.Tex);
	float depth = gDepthRT.Sample(samLinear, pin.Tex).r;	// 뎁스가 채널에 들어있다.
	float viewSpaceZ = DepthBufferConvertToLinear(depth);

	if (depth < 1.0f)
	{
		float percent = saturate((viewSpaceZ - gDepthStart) / gDepthRange);

		// 보간한다. (그냥 찍은 화면과 블러 처리된 화면을)
		color = lerp(color, blurColor, percent);
	}

	color.a = 1.0f;

    return color;
}

technique11 DOF
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}