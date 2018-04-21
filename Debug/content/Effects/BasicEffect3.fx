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
	OUT.Position.y *= sin(CameraPos.z);

	return OUT;
}

/************* Pixel Shader *************/

float4 pixel_shader(VS_OUTPUT IN) : SV_Target
{
	//float4 OUT = (float4)0;

	//float3 normal = normalize(IN.Normal);
	//float3 lightDirection = normalize(IN.LightDirection);
	//float n_dot_1 = dot(lightDirection, normal);

	//float4 color = ColorTexture.Sample(ColorSampler, IN.TextureCoordinate);
	//float3 ambient = AmbientColor.rgb * AmbientColor.a * color.rgb;

	//float3 diffuse = (float3)0;

	/*if (n_dot_1 > 0)
	{
		diffuse = LightColor.rgb * LightColor.a * n_dot_1 * color.rgb;
	}*/

	//OUT.rgb = ambient + diffuse;
	//OUT.a = color.a;
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