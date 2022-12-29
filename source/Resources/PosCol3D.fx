float4x4 gWorldviewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;

//Different Sampler states
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState samAnisptropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};


//Vertex Input
struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float2 TexCoord : TEXCOORD;
};
//Vertex Output
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Color : COLOR;
	float2 TexCoord : TEXCOORD;
};

//Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1), gWorldviewProj);
	output.Color = input.Color;
	output.TexCoord = input.TexCoord;
	return output;
}

//PixelShaders

//float4 PS(VS_OUTPUT input) : SV_TARGET
//{
//	return gDiffuseMap.Sample(samPoint, input.TexCoord);
//}

float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(input, samPoint);
}

float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(input, samLinear);
}

float4 PSAnisptropic(VS_OUTPUT input) : SV_TARGET
{

	return gDiffuseMap.Sample(input, samAnisptropic);
}

technique11 DefaultTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}
