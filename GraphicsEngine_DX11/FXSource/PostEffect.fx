//--------------------------------------------------
//	PostEffect.fx
//
//	����Ʈ ����Ʈ�� ���̴�
//
//	������ ��ó���� ���� (Bloom)
//
//	2021.06.22 LeHideKGIT
//--------------------------------------------------


// cross type, size 3
static float3 g_Filter[12] =
{
    -3, 0, 0.111, -2, 0, 0.250, -1, 0, 1.000,
	+3, 0, 0.111, +2, 0, 0.250, +1, 0, 1.000,
	
	 0, -3, 0.111, 0, -2, 0.250, 0, -1, 1.000,
	 0, +3, 0.111, 0, +2, 0.250, 0, +1, 1.000
};

cbuffer cbPerObject
{
	float4x4 gTexTransform;
	float4x4 gWorldViewProj;
}; 

float2 g_TexelSize;

float g_BloomWeight = 6.0f / 32.0f;
float g_BloomHue = 2.25f;

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;

SamplerState samLinear
{
	//Filter = ANISOTROPIC;
	//MaxAnisotropy = 4;

	Filter = MIN_MAG_MIP_LINEAR;

	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 Tex     : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	return vout;
}
 
float4 PS(VertexOut pin) : SV_Target
{
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);

	// Sample texture.
	texColor = gDiffuseMap.Sample( samLinear, pin.Tex );

    return texColor;
}

//--------------------------------------------------
// Pixel Shader - Simple Blur
//--------------------------------------------------
float4 PS_Blur(VertexOut pin) : SV_Target
{
    float4 AverageColor = 0;
    float4 Pixel = 0;

    for (int i = 0; i < 12; ++i)
    {
		// �ֺ� ������ �����´�. 12���� ���� ��ճ��� �ʹ�.
        Pixel = gDiffuseMap.Sample(samLinear, pin.Tex + (g_Filter[i].xy) * g_TexelSize) * 1.0f / 12.0f;
		
		// 0���� 1�� ������ �ٲ� �ش�.
		// Clamps the specified value within the range of 0 to 1.
        Pixel = saturate(Pixel);
		
		// 12���� ��������� ���� �ʹ�.
        AverageColor += Pixel;
    }

    return AverageColor;
}


//--------------------------------------------------
// Pixel Shader - Bloom type 2 (+Blur)
//--------------------------------------------------
float4 PS_Bloom2(VertexOut pin) : SV_Target
{
    float4 AddColor = 0;
    float4 Pixel = 0;

    for (int i = 0; i < 12; ++i)
    {
		// ����ġ ���ø��� �ϸ�, ��Ⱑ �����ϰ� ���ȴ�. �̰��� ������ ������, saturate�ϸ� ���� �κи� �߷�����.
        Pixel = gDiffuseMap.Sample(samLinear, pin.Tex + (g_Filter[i].xy) * g_TexelSize * 2.0f) - 0.5f;
		
		// Clamps the specified value within the range of 0 to 1.
        Pixel = saturate(Pixel);
		
		// �׷��� ������ ���� �κ��� ����ġ�� �� �ش�.
        AddColor += Pixel * g_BloomWeight;

		/// Bloom Hue
		/*
		Pixel.a = Pixel.r + Pixel.g + Pixel.b;
		
		if( Pixel.a > g_BloomHue )
		{
			Pixel.r *= Pixel.r;
			Pixel.g *= Pixel.g;
			Pixel.b *= Pixel.b;
		
			AddColor += Pixel * g_BloomWeight;
		}
		*/
    }
	
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);

	// �ؽ��Ĵ�� �׸���
	// Sample texture.
    //texColor = gDiffuseMap.Sample(samAnisotropic, pin.Tex);
	
	// �߰����� ���� �κ��� ������ �ȼ��� �����ش�.
    //texColor = AddColor;
    texColor = gDiffuseMap.Sample(samLinear, pin.Tex) + AddColor;

    return texColor;
}

technique11 PPStandard
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );
    }
}

technique11 PPBloom2
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Bloom2()));
    }
}

technique11 PPBlur
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS_Blur()));
    }
}