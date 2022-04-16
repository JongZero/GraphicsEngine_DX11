//=============================================================================
	// 모션 블러
//=============================================================================

uint gSamples = 10;

float4x4 gWorldViewProj;
float4x4 gViewProjInv;
float4x4 gViewProj_Prev;		// 직전 프레임의 view proj 행렬

Texture2D gFinal;
Texture2D gDepthMap;

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	vout.Tex = vin.Tex;

	return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
	// Get the depth buffer value at this pixel.    
	float zOverW = gDepthMap.Sample(samLinear, pin.Tex).z;

	// H is the viewport position at this pixel in the range -1 to 1.
	float4 H = float4(pin.Tex.x * 2 - 1, (1 - pin.Tex.y) * 2 - 1, zOverW, 1);
	
	// Transform by the view-projection inverse.    
	float4 D = mul(H, gViewProjInv); 
	
	// Divide by w to get the world position.    
	float4 worldPos = D / D.w; 

	// Current viewport position    
	float4 currentPos = H; 
	
	// Use the world position, and transform by the previous view-    
	// projection matrix.    
	float4 previousPos = mul(worldPos, gViewProj_Prev); 
	
	// Convert to nonhomogeneous points [-1,1] by dividing by w. 
	previousPos /= previousPos.w; 
	
	// Use this frame's position and last frame's to compute the pixel    
	// velocity.    
	//float f = 1.0f / gSamples;
	float f = 1.0f / 4;
	float2 velocity = (currentPos - previousPos) * f;

	// Get the initial color at this pixel.    
	float4 color = gFinal.Sample(samLinear, pin.Tex);
	pin.Tex += velocity;
	
	for(int i = 1; i < gSamples; ++i, pin.Tex += velocity)
	{   
		// Sample the color buffer along the velocity vector.    
		float4 currentColor = gFinal.Sample(samLinear, pin.Tex);
		
		// Add the current color to our color sum.   
		color += currentColor; 
	} 
	
	// Average all of the samples to get the final blur color.    
	float4 finalColor = color / gSamples;

	return finalColor;
}

technique11 MotionBlur
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
