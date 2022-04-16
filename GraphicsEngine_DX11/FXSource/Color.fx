//***************************************************************************************
// color.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Transforms and colors geometry.
//***************************************************************************************

// 상수 버퍼
cbuffer cbPerObject
{
	float4x4 gWorldViewProj;    // 한 점을 국소 공간에서 동차 절단 공간으로 변환하는데 쓰이는, 세계, 시야, 투영 행렬을 하나로 결합한 것이다.
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
	
	// Transform to homogeneous clip space. 정점을 국소 공간에서 동차 절단 공간으로 변환한다.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Just pass vertex color into the pixel shader. 정점 색상을 그대로 픽셀 셰이더에 전달한다.
    vout.Color = vin.Color;
    
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    return pin.Color;
}


// 효과 파일(.fx) 안에서 래스터화기 상태 집합을 만들고 설정하는 예
RasterizerState WireframeRS
{
    FillMode = Wireframe;       // 접두사 D3D11_FILL이 빠짐
    CullMode = Back;            // 접두사 D3D11_CULL이 빠짐
    FrontCounterClockwise = false;

    // 명시적으로 설정하지 않은 속성들에 대해서는 기본값이 쓰인다.
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