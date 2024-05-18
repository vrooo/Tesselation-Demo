cbuffer cbView : register(b0) //Vertex Shader constant buffer slot 0
{
	matrix viewMatrix;
};

cbuffer cbProj : register(b1) //Vertex Shader constant buffer slot 1
{
	matrix projMatrix;
};

struct VSInput
{
	float3 pos : POSITION;
};

struct HSInput
{
	float4 pos : SV_POSITION;
};

HSInput main(VSInput i)
{
	HSInput o;
	o.pos = mul(viewMatrix, float4(i.pos, 1.0f));
	o.pos = mul(projMatrix, o.pos);
	return o;
}