cbuffer cbSurfaceColor : register(b0) //Pixel Shader constant buffer slot 0
{
	float4 surfaceColor;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float3 world : POSITION0;
	float3 norm : NORMAL0;
	float3 tangent : NORMAL1;
	float3 bitangent : NORMAL2;
	float3 view : TEXCOORD0;
	float2 tex : TEXCOORD1;
};

float4 main(PSInput i) : SV_TARGET
{
	return surfaceColor;
}