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

float smootherstep(float inMin, float inMax, float value)
{
	float v = clamp((value - inMin) / (inMax - inMin), 0.0, 1.0);
	return v * v * v * (v * (v * 6.0 - 15.0) + 10.0);
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
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	// SDF
	float minThreshold = 0.725;
	float maxThreshold = 0.75;
	float distance = diffuseTexture.Sample(diffuseSampler, input.uv).a;
	float alpha = smootherstep(minThreshold, maxThreshold, distance);
	float4 fontColor = float4(1.0, 1.0, 1.0, alpha) * input.color;

	// drop shadow
	float2 shadowOffset = float2(-2.0, 2.0);
	float4 shadowColor = float4(0.0, 0.0, 0.0, 1.0);
	float2 textureDimensions;
	diffuseTexture.GetDimensions(textureDimensions.x, textureDimensions.y);
	float4 shadow = diffuseTexture.Sample(diffuseSampler, input.uv + shadowOffset / textureDimensions);
	float4 color = fontColor;
	if (distance > 0.15 && distance < 0.7)
	{
		color = lerp(fontColor, shadowColor, shadow.a);
	}
	
	clip(color.a - 0.01);
	return color;
}