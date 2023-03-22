Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

cbuffer CameraConstant : register(b2)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
};

cbuffer ModelConstant : register(b3)
{
	float4x4 ModelMatrix;
	float4 ModelColor;
};

cbuffer BorderConstant : register(b4)
{
	float2 BoxMins;
	float2 BoxMaxs;
	float  Radius;
	float3 padding3;
};

struct vs_input_t
{
	float3 localPosition : POSITION;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct v2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
	float3 localPosition : POSITION;
};

float GetLength(float2 displacement)
{
	return sqrt(displacement.x * displacement.x + displacement.y * displacement.y);
}

bool IsOutsideFrame(float2 position)
{
	float2 bottomLeft = float2(BoxMins.x + Radius, BoxMins.y + Radius);
	float2 bottomRight = float2(BoxMaxs.x - Radius, BoxMins.y + Radius);
	float2 topRight = float2(BoxMaxs.x - Radius, BoxMaxs.y - Radius);
	float2 topLeft = float2(BoxMins.x + Radius, BoxMaxs.y - Radius);

	if (position.x < bottomLeft.x && position.y < bottomLeft.y)
	{
		float dispBL = GetLength(position - bottomLeft);
		if (dispBL > Radius) return true;
	}
	else if (position.x > bottomRight.x && position.y < bottomRight.y)
	{
		float dispBR = GetLength(position - bottomRight);
		if (dispBR > Radius) return true;
	}
	else if (position.x > topRight.x && position.y > topRight.y)
	{
		float dispTR = GetLength(position - topRight);
		if (dispTR > Radius) return true;
	}
	else if (position.x < topLeft.x && position.y > topLeft.y)
	{
		float dispTL = GetLength(position - topLeft);
		if (dispTL > Radius) return true;
	}
	return false;
}

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	v2p.position = clipPosition;
	v2p.color = input.color * ModelColor;
	v2p.uv = input.uv;
	v2p.localPosition = input.localPosition;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 color = diffuseTexture.Sample(diffuseSampler, input.uv) * input.color;
	if (IsOutsideFrame(input.localPosition.xy)) color.a = 0.f;
	clip(color.a - 0.01f);
	return color;
}