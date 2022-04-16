//=============================================================================
// Fire.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Fire particle system.  Particles are emitted directly in world space.
//=============================================================================


//***********************************************
// GLOBALS                                      *
//***********************************************

cbuffer cbPerFrame
{
	float3 gEyePosW;
	
	// for when the emit position/direction is varying
	// 방출 위치나 방향이 다른 경우를 위한 변수들
	float3 gEmitPosW;
	float3 gEmitDirW;
	
	float gGameTime;
	float gTimeStep;
	float4x4 gViewProj; 
};

cbuffer cbFixed
{
	// Net constant acceleration used to accerlate the particles.
	// 입자의 가속을 위한 알짜 상수 가속도
	float3 gAccelW = {0.0f, 7.8f, 0.0f};
	
	// Texture coordinates used to stretch texture over quad 
	// when we expand point particle into a quad.
	// 텍스쳐를 사각형 전체에 입히는 텍스쳐 좌표들
	// 점 입자를 사각형으로 확장할 때 쓰임
	float2 gQuadTexC[4] = 
	{
		float2(0.0f, 1.0f),
		float2(1.0f, 1.0f),
		float2(0.0f, 0.0f),
		float2(1.0f, 0.0f)
	};
};
 
// Array of textures for texturing the particles.
// 입자 텍스쳐 적용을 위한 텍스쳐 배열
Texture2DArray gTexArray;

// Random texture used to generate random numbers in shaders.
// 쉐이더에서 난수를 얻는 데 쓰이는 무작위 텍스쳐
Texture1D gRandomTex;
 
SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};
 
DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

DepthStencilState NoDepthWrites
{
    DepthEnable = TRUE;
    DepthWriteMask = ZERO;
};

BlendState AdditiveBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

//***********************************************
// HELPER FUNCTIONS                             *
//***********************************************
float3 RandUnitVec3(float offset)
{
	// Use game time plus offset to sample random texture.
	// 게임 시간 더하기 오프셋을 무작위 텍스쳐 추출을 위한 텍스쳐 좌표로 사용한다.
	float u = (gGameTime + offset);
	
	// coordinates in [-1,1]
	// 벡터 성분들의 범위는 [-1,1]이다.
	// 
	// SampleLevel 함수 : 표본을 추출할 밉맵 수준을 명시적으로 지정할 수 있다.
	// SampleLevel 파라미터 : 
	// (표본추출기 객체, 
	// 텍스쳐 좌표(1차원 텍스쳐의 경우 성분 하나), 
	// 밉맵 수준(밉맵 수준이 단 하나인 텍스쳐의 경우 반드시 0을 지정해야 한다.))
	float3 v = gRandomTex.SampleLevel(samLinear, u, 0).xyz;
	
	// project onto unit sphere
	// 단위 구로 투영한다.
	return normalize(v);
}
 
//***********************************************
// STREAM-OUT TECH                              *
//***********************************************

#define PT_EMITTER 0
#define PT_FLARE 1
 
struct Particle
{
	float3 InitialPosW : POSITION;
	float3 InitialVelW : VELOCITY;
	float2 SizeW       : SIZE;
	float Age          : AGE;
	uint Type          : TYPE;
};
  
Particle StreamOutVS(Particle vin)
{
	return vin;
}

// The stream-out GS is just responsible for emitting 
// new particles and destroying old particles.  The logic
// programed here will generally vary from particle system
// to particle system, as the destroy/spawn rules will be 
// different.
// 스트림 출력 전용 기하 쉐이더는 새 입자의 방출과 기존 입자의 파괴만 담당한다.
// 입자 시스템마다 입자의 생성·파괴 규칙이 다를 것이므로,
// 이 부분의 구현 역시 입자 시스템마다 다를 필요가 있다.
[maxvertexcount(2)]
void StreamOutGS(point Particle gin[1], 
                 inout PointStream<Particle> ptStream)
{	
	gin[0].Age += gTimeStep;
	
	if( gin[0].Type == PT_EMITTER )
	{	
		// time to emit a new particle?
		// 새 입자를 방출할 시간이 되었는가?
		if( gin[0].Age > 0.005f )
		{
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;
			
			Particle p;
			p.InitialPosW = gEmitPosW.xyz;
			p.InitialVelW = 4.0f*vRandom;
			p.SizeW       = float2(3.0f, 3.0f);
			p.Age         = 0.0f;
			p.Type        = PT_FLARE;
			
			ptStream.Append(p);
			
			// reset the time to emit
			// 방출할 시간을 재설정한다.
			gin[0].Age = 0.0f;
		}
		
		// always keep emitters
		// 방출기 입자 하나는 항상 유지한다.
		ptStream.Append(gin[0]);
	}
	else
	{
		// Specify conditions to keep particle; this may vary from system to system.
		// 여기서 입자를 유지할 조건들을 지정한다.
		// 구체적인 조건은 입자 시스템마다 다를 수 있다.
		if( gin[0].Age <= 1.0f )
			ptStream.Append(gin[0]);
	}		
}

// 스트림 출력을 위해서는 기하 셰이더를 특별한 형태로 작성해야 한다.
GeometryShader gsStreamOut = ConstructGSWithSO( 
	CompileShader( gs_5_0, StreamOutGS() ), 
	"POSITION.xyz; VELOCITY.xyz; SIZE.xy; AGE.x; TYPE.x" );
	
technique11 StreamOutTech
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, StreamOutVS() ) );
        SetGeometryShader( gsStreamOut );
        
        // disable pixel shader for stream-out only
		// 스트림 출력 전용을 위해 픽셀 셰이더를 비활성화한다.
        SetPixelShader(NULL);
        
        // we must also disable the depth buffer for stream-out only
		// 스트림 출력 전용을 위해서는 깊이 버퍼도 비활성화해야 한다.
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//***********************************************
// DRAW TECH                                    *
//***********************************************

struct VertexOut
{
	float3 PosW  : POSITION;
	float2 SizeW : SIZE;
	float4 Color : COLOR;
	uint   Type  : TYPE;
};

VertexOut DrawVS(Particle vin)
{
	VertexOut vout;
	
	float t = vin.Age;
	
	// constant acceleration equation
	// 상수 가속 공식
	vout.PosW = 0.5f*t*t*gAccelW + t*vin.InitialVelW + vin.InitialPosW;
	
	// fade color with time
	// 시간에 따른 색상 감소
	float opacity = 1.0f - smoothstep(0.0f, 1.0f, t/1.0f);
	vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);
	
	vout.SizeW = vin.SizeW;
	vout.Type  = vin.Type;
	
	return vout;
}

struct GeoOut
{
	float4 PosH  : SV_Position;
	float4 Color : COLOR;
	float2 Tex   : TEXCOORD;
};

// The draw GS just expands points into camera facing quads.
// 렌더링용 기하 쉐이더는 그냥 점을 카메라를 향한 사각형으로 확장한다.
[maxvertexcount(4)]
void DrawGS(point VertexOut gin[1], 
            inout TriangleStream<GeoOut> triStream)
{	
	// do not draw emitter particles.
	// 방출기 입자는 그리지 않는다.
	if( gin[0].Type != PT_EMITTER )
	{
		//
		// Compute world matrix so that billboard faces the camera.
		// 빌보드가 카메라를 향하게 하는 세계 행렬을 계산한다.
		//
		float3 look  = normalize(gEyePosW.xyz - gin[0].PosW);
		float3 right = normalize(cross(float3(0,1,0), look));
		float3 up    = cross(look, right);
		
		//
		// Compute triangle strip vertices (quad) in world space.
		// 사각형을 구성하는 삼각형 띠 정점들을 계산한다.
		//
		float halfWidth  = 0.5f*gin[0].SizeW.x;
		float halfHeight = 0.5f*gin[0].SizeW.y;
	
		float4 v[4];
		v[0] = float4(gin[0].PosW + halfWidth*right - halfHeight*up, 1.0f);
		v[1] = float4(gin[0].PosW + halfWidth*right + halfHeight*up, 1.0f);
		v[2] = float4(gin[0].PosW - halfWidth*right - halfHeight*up, 1.0f);
		v[3] = float4(gin[0].PosW - halfWidth*right + halfHeight*up, 1.0f);
		
		//
		// Transform quad vertices to world space and output 
		// them as a triangle strip.
		// 이 정점들을 세계 공간으로 변환하고, 하나의 삼각형 띠로서 출력한다.
		//
		GeoOut gout;
		[unroll]
		for(int i = 0; i < 4; ++i)
		{
			gout.PosH  = mul(v[i], gViewProj);
			gout.Tex   = gQuadTexC[i];
			gout.Color = gin[0].Color;
			triStream.Append(gout);
		}	
	}
}

float4 DrawPS(GeoOut pin) : SV_TARGET
{
	return gTexArray.Sample(samLinear, float3(pin.Tex, 0))*pin.Color;
}

technique11 DrawTech
{
    pass P0
    {
        SetVertexShader(   CompileShader( vs_5_0, DrawVS() ) );
        SetGeometryShader( CompileShader( gs_5_0, DrawGS() ) );
        SetPixelShader(    CompileShader( ps_5_0, DrawPS() ) );
        
        SetBlendState(AdditiveBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xffffffff);
        SetDepthStencilState( NoDepthWrites, 0 );
    }
}