struct VS_INPUT
{
	Vector3 Position = POSITION;
	Vector3 Color = COLOR;
};

struct VS_OUTPUT
{
	Vector4 Position = SV_POSITION;
	Vector3 Color = COLOR;
};

VS_OUTPUT VertexShader(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT);
	output.Position = Vector4(input.Position, 1);
	output.Color = input.Color;
	return output;
}

Vector4 Renderer::PixelShader(VS_OUTPUT input)
{
	return Vector4(input.Color, 1);
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
