//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

// ��� ����
cbuffer cbPerObject
{
	float4x4 gWorldViewProj;    // �� ���� ���� �������� ���� ���� �������� ��ȯ�ϴµ� ���̴�, ����, �þ�, ���� ����� �ϳ��� ������ ���̴�.
};

struct VertexIn
{
	float3 PosL  : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float4 Color : COLOR;
};

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

DepthStencilState NoDepth
{
    DepthEnable = FALSE;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to homogeneous clip space. ������ ���� �������� ���� ���� �������� ��ȯ�Ѵ�.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader. ���� ������ �״�� �ȼ� ���̴��� �����Ѵ�.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


// ȿ�� ����(.fx) �ȿ��� ������ȭ�� ���� ������ ����� �����ϴ� ��
RasterizerState WireframeRS
{
    FillMode = Wireframe;       // ���λ� D3D11_FILL�� ����
    CullMode = Back;            // ���λ� D3D11_CULL�� ����
    FrontCounterClockwise = false;

    // ��������� �������� ���� �Ӽ��鿡 ���ؼ��� �⺻���� ���δ�.
};

technique11 ColorTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS() ) );

        SetRasterizerState(NoCull);
        SetDepthStencilState(LessEqualDSS, 0);
        SetRasterizerState(WireframeRS);
    }
}

technique11 Color_NoDepthTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS()));

        SetRasterizerState(NoCull);
        SetDepthStencilState(NoDepth, 0);
        SetRasterizerState(WireframeRS);
    }
}