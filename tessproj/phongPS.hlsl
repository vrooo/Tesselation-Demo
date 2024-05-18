cbuffer cbLightPos : register(b1) //Pixel Shader constant buffer slot 1
{
	float4 lightPos;
};

static const float3 ambientColor = float3(0.2f, 0.2f, 0.2f);
static const float3 lightColor = float3(1.0f, 1.0f, 1.0f);
static const float kd = 0.5, ks = 0.4f, m = 200.0f;

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

Texture2D diffuseMap : register(t0);
Texture2D normalMap : register(t1);
SamplerState samp : register(s0);

float4 main(PSInput i) : SV_TARGET
{
	float4 surfaceColor = diffuseMap.Sample(samp, i.tex);
	float3 normalTex = normalMap.Sample(samp, i.tex).rgb - 0.5f;
	float3 normal = normalize(normalTex.x * i.tangent + normalTex.y * i.bitangent + normalTex.z * i.norm);

	float3 color = surfaceColor.rgb * ambientColor; // ambient
	float3 lightPosition = lightPos.xyz;
	float3 lightVec = normalize(lightPosition - i.world);
	float3 halfVec = normalize(i.view + lightVec);
	float nl = dot(normal, lightVec);
	if (nl > 0)
	{
		color += lightColor * surfaceColor.xyz * kd * saturate(nl); // diffuse
		float nh = dot(normal, halfVec);
		nh = saturate(nh);
		nh = pow(nh, m);
		nh *= ks;
		color += lightColor * nh; // specular
	}

	return float4(saturate(color), surfaceColor.a);
}