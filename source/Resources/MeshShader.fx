//Matrices
float4x4 gWorldviewProj : WorldViewProjection;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gInvViewMatrix : ViewInverseMatrix;


//Texture maps
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

//Light
float3 gLightdirection = normalize(float3(0.577f, -0.577f, 0.577f));

//Extra Data
float gPi = 3.14159265359f;
float gLightIntensity = 7.f;
float gShininess = 25.0f;

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

SamplerState samAnisotropic
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
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

//Vertex Output
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : W_POSITION;
	float3 Color : COLOR;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

//Vertex Shader
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1), gWorldviewProj);
	output.Color = input.Color;
	output.TexCoord = input.TexCoord;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorldMatrix);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorldMatrix);
	output.WorldPosition = mul(float4(input.Position, 1), gWorldMatrix);
	return output;
}

//Shaders
float4 Lambert(float kd, const float4 ColorRGB)
{
	return (kd * ColorRGB) / gPi;
}

float4 Phong(float ks, float exp, float3 light, float3 viewDirection, float3 normal)
{
	float3 reflected = reflect(light, normal);

	float angle = saturate(dot(reflected, viewDirection));

	float specularReflection = ks * pow(angle, exp);
	float4 Color = float4( specularReflection ,specularReflection ,specularReflection , 1 );

	return Color;
}


//PixelShaders
float4 Sampler(VS_OUTPUT input, SamplerState sampleVar)
{

	float3 pixelNormal = input.Normal ;
	//Normal calculations

	float3 binormal = cross(input.Normal, input.Tangent);

	float4x4 tangentSpaceAxis = float4x4(
		float4(input.Tangent, 0.0f), 
		float4(binormal, 0.f), 
		float4(input.Normal, 0.f), 
		float4(0.f, 0.f, 0.f, 1.f)
		);


	float3 sampledNormal = gNormalMap.Sample(sampleVar, input.TexCoord).rgb; 
	sampledNormal = (2.f * sampledNormal) - float3( 1.f, 1.f, 1.f); // [0, 1] -> [-1, 1]
	float3 sampledNormalVector = float3( sampledNormal.r, sampledNormal.g, sampledNormal.b);
	pixelNormal = mul(float4(sampledNormalVector, 0), tangentSpaceAxis);


	float observedArea = saturate(dot(-gLightdirection, pixelNormal));
	float3 viewDirection = normalize(input.WorldPosition.xyz - gInvViewMatrix[3].xyz);

	const float4 lambert = Lambert(1, gDiffuseMap.Sample(sampleVar, input.TexCoord)) ;
	const float phongExponent = gGlossinessMap.Sample(sampleVar, input.TexCoord) * gShininess;

	const float4 specular = gSpecularMap.Sample(sampleVar,input.TexCoord) * Phong(1.0f, phongExponent, -gLightdirection, viewDirection, pixelNormal);

	return (gLightIntensity * lambert + specular) * observedArea;

}

float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
	return Sampler(input, samPoint);
}

float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
	return Sampler(input, samLinear);
}

float4 PSAnisptropic(VS_OUTPUT input) : SV_TARGET
{
	return Sampler(input, samAnisotropic);
}

technique11 AnisotropicTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisptropic()));
	}
}

technique11 PointTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPoint()));
	}
}

technique11 LinearTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
}




