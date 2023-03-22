Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

cbuffer CameraConstant : register(b2)
{
	float OrthoMinX;
	float OrthoMinY;
	float OrthoMinZ;
	float padding0;
	float OrthoMaxX;
	float OrthoMaxY;
	float OrthoMaxZ;
	float padding1;
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
};

float RangeMap(float input, float inMin, float inMax, float outMin, float outMax)
{
	float fraction;
	if (inMin == inMax)
	{
		fraction = 0.5f;
	}
	else
	{
		fraction = (input - inMin) / (inMax - inMin);
	}
	return fraction * (outMax - outMin) + outMin;
}

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	v2p.position = float4(input.localPosition, 1);
	v2p.position.x = RangeMap(v2p.position.x, OrthoMinX, OrthoMaxX, -1.f, 1.f);
	v2p.position.y = RangeMap(v2p.position.y, OrthoMinY, OrthoMaxY, -1.f, 1.f);
	v2p.position.z = RangeMap(v2p.position.z, OrthoMinZ, OrthoMaxZ, 0.f, 1.f);
	v2p.color = input.color;
	v2p.uv = input.uv;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 color = diffuseTexture.Sample(diffuseSampler, input.uv);
	return color * input.color;
}