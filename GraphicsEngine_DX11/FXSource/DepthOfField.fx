Texture2D SrcTex;
Texture2D DOFBlurTex;
Texture2D DepthTex;

SamplerState PointSampler
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;

    AddressU = Clamp;
    AddressV = Clamp;

};

SamplerState LinearSampler
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;

    AddressU = Wrap;
    AddressV = Wrap;
};

cbuffer cbPerFrame
{
    float2 ProjValues;
    float2 DOFFarValues;
};

cbuffer cbPerObject
{
    float4x4 gWorldViewProj;
};

//-----------------------------------------------------------------------------------------
// Vertex shader
//-----------------------------------------------------------------------------------------
struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 Tex : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to homogeneous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);

    vout.Tex = vin.Tex;

    return vout;
}

//-----------------------------------------------------------------------------------------
// Pixel shader
//-----------------------------------------------------------------------------------------
float ConvertZToLinearDepth(float depth)
{
    float linearDepth = ProjValues.x / (depth + ProjValues.y);
    return linearDepth;
}

float3 DistanceDOF(float3 colorFocus, float3 colorBlurred, float depth)
{
	// Find the depth based blur factor
    float blurFactor = saturate((depth - DOFFarValues.x) * DOFFarValues.y);

	// Lerp with the blurred color based on the CoC factor
    return lerp(colorFocus, colorBlurred, blurFactor);
}

float4 FinalPassPS(VertexOut In) : SV_TARGET
{
	// Get the color and depth samples
    float3 color = SrcTex.Sample(PointSampler, In.Tex.xy).xyz;

	// Distance DOF only on pixels that are not on the far plane
    float depth = DepthTex.Sample(PointSampler, In.Tex.xy).z;
    if (depth < 1.0)
    {
		// Get the blurred color from the down scaled HDR texture
        float3 colorBlurred = DOFBlurTex.Sample(LinearSampler, In.Tex.xy).xyz;

		// Convert the full resulotion depth to linear depth
        depth = ConvertZToLinearDepth(depth);

		// Compute the distance DOF color
        color = DistanceDOF(color, colorBlurred, depth);
    }

	// Output the LDR value
    return float4(color, 1.0);
}

technique11 DOFTech
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, FinalPassPS()));
    }
}