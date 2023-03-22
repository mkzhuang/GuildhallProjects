Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

cbuffer LightConstant : register(b1)
{
	float3 SunDirection;
	float SunIntensity;
	float3 SunColor;
	float AmbientIntensity;
};

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
	float3 localNormal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

struct v2p_t
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	float4 localPosition = float4(input.localPosition, 1);
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	float4 localNormal = float4(input.localNormal, 0);
	float4 worldNormal = mul(ModelMatrix, localNormal);

	v2p.position = clipPosition;
	v2p.normal = worldNormal.xyz;
	v2p.color = input.color * ModelColor;
	v2p.uv = input.uv;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float directional = SunIntensity * saturate(dot(normalize(input.normal), -SunDirection));
	//float4 lightColor = float4(SunColor.xyz, AmbientIntensity + direction);
	float4 lightColor = float4((AmbientIntensity + directional).xxx, 1);
	float4 color = diffuseTexture.Sample(diffuseSampler, input.uv) * input.color * lightColor;
	clip(color.a - 0.01f);
	return color;
}