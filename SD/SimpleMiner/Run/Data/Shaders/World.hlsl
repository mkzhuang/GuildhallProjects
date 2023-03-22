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

cbuffer GameConstant : register(b4)
{
	float4 CameraWorldPosition;
	float4 SkyColor;
	float4 OutdoorLightColor;
	float4 IndoorLightColor;
	float  FogStartDistance;
	float  FogEndDistance;
	float  FogMaxAlpha;
	float  CurrentTime;
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
	float4 worldPosition : WORLDPOSITION;
};

float3 DiminishingAdd(float3 a, float3 b)
{
	return 1.f - ((1.f - a) * (1.f - b));
}

float Hash(float n)
{
	return frac(cos(n) * 1897.196f);
}

float RangeMap(float value, float inMin, float inMax, float outMin, float outMax)
{
	float clamped = saturate(value);
	float fraction = (clamped - inMin) / (inMax - inMin);
	return fraction * (outMax - outMin) + outMin;
}

float RandomNoise(float2 xy)
{
	float t = RangeMap(sin(xy.x + 1.7f) + 0.5f * cos(xy.y + 2.9f), -1.5f, 1.5f, -1.f, 1.f);
	return t;
}

v2p_t VertexMain(vs_input_t input)
{
	v2p_t v2p;
	float4 localPosition = float4(input.localPosition, 1.f);
	localPosition.z = localPosition.z - input.color.b * (0.25f + 0.25f * RandomNoise(localPosition.xy + 300.f * float2(CurrentTime, CurrentTime)));
	float4 worldPosition = mul(ModelMatrix, localPosition);
	float4 viewPosition = mul(ViewMatrix, worldPosition);
	float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	v2p.position = clipPosition;
	v2p.color = input.color * ModelColor;
	v2p.uv = input.uv;
	v2p.worldPosition = worldPosition;
	return v2p;
}

float4 PixelMain(v2p_t input) : SV_Target0
{
	float4 color = diffuseTexture.Sample(diffuseSampler, input.uv);
	clip(color.a - 0.01f);

	float outdoorLightLevel = input.color.r;
	float indoorLightLevel = input.color.g;
	float3 outdoorLight = outdoorLightLevel * OutdoorLightColor.rgb;
	float3 indoorLight = indoorLightLevel * IndoorLightColor.rgb;
	float3 diffuseLight = DiminishingAdd(outdoorLight, indoorLight);
	float3 diffuseColor = diffuseLight * color.rgb;

	float3 cameraToPixelDisplacement = input.worldPosition.xyz - CameraWorldPosition.xyz;
	float cameraToPixelDistance = length(cameraToPixelDisplacement);
	float fogDensity = FogMaxAlpha * saturate((cameraToPixelDistance - FogStartDistance) / (FogEndDistance - FogStartDistance));
	float3 outputColor = lerp(diffuseColor, SkyColor.rgb, fogDensity);
	float outputAlpha = input.color.a * saturate(color.a + fogDensity);

	return float4(outputColor, outputAlpha);
}

