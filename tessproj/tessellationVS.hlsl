cbuffer cbView : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix viewMatrix;
};

struct VSInput
{
	float3 pos : POSITION;
};

struct HSInput
{
	float4 pos : SV_POSITION;
	float3 cam : POSITION;
};

HSInput main(VSInput i)
{
	HSInput o;
	o.pos = float4(i.pos, 1.0f);
	o.cam = mul(viewMatrix, o.pos).xyz;
	return o;
}