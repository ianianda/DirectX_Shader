/************* Resources *************/

cbuffer CBufferPerObject
{
	float4x4 WorldViewProjection : WORLDVIEWPROJECTION;
	float3 CameraPos : CAMERAPOS;
}

/************* Data Structures *************/

struct VS_INPUT
{
	float4 ObjectPosition: POSITION;
	float4 Color : COLOR;
};

struct VS_OUTPUT
{
	float4 Position: SV_Position;
	float4 Color : COLOR;
};

RasterizerState DisableCulling
{
	CullMode = NONE;
};

/************* Vertex Shader *************/

VS_OUTPUT vertex_shader(VS_INPUT IN)
{
	VS_OUTPUT OUT = (VS_OUTPUT)0;

	OUT.Position = mul(IN.ObjectPosition, WorldViewProjection);
	OUT.Color = IN.Color;
	//OUT.Color = (float4)(IN.Color.a - IN.Color.rgb, IN.Color.a);
	OUT.Position.x *= sin(CameraPos.z);

	return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN) : SV_Target
{
	//return IN.Color.rbga;
	//IN.Color = (float4)(0,1,0,0,1);
	//IN.Color = (float4)(IN.Color.a - IN.Color.rgb, IN.Color.a);
	//IN.Color = (float4)(IN.Color.b, IN.Color.r, IN.Color.g, IN.Color.a);
	//IN.Color = (float4)(sin(IN.Color.r)*2,cos(IN.Color.g),pow(IN.Color.b,2),IN.Color.a);
	//IN.Color.b = IN.Color.r;
	//IN.Color.r = IN.Color.g;
	IN.Color.r = sin(IN.Color.r) * sin(IN.Color.g);
	IN.Color.g = cos(IN.Color.g) * cos(IN.Color.g);
	IN.Color.b = pow(IN.Color.b, 2) * cos(IN.Color.g);
	IN.Color.a = sin(IN.Color.a);
	return IN.Color;
}

/************* Techniques *************/

technique11 main11
{
	pass p0
	{
		SetVertexShader(CompileShader(vs_5_0, vertex_shader()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, pixel_shader()));

		SetRasterizerState(DisableCulling);
	}
}