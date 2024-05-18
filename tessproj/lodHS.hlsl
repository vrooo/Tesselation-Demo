#define INPUT_PATCH_SIZE 16
#define INPUT_PATCH_EDGE 4
#define OUTPUT_PATCH_SIZE 16

cbuffer cbTessCount : register(b0)
{
	float4 tessCount;
};

struct HSInput
{
	float4 pos : SV_POSITION;
	float3 cam : POSITION;
};

struct HSPatchOutput
{
	float edges[4] : SV_TessFactor;
	float inside[2] : SV_InsideTessFactor;
};

struct DSControlPoint
{
	float4 pos : SV_POSITION;
};

float factor(float z)
{
	return max(-4.f * log10(0.01f * z), 1.f);
}

HSPatchOutput HS_Patch(InputPatch<HSInput, INPUT_PATCH_SIZE> ip, uint patchId : SV_PrimitiveID)
{
	HSPatchOutput o;
	float zIn = 0;
	int countedIn = 0;
	for (int i = 0; i < INPUT_PATCH_SIZE; i++)
	{
		float ipz = ip[i].cam.z;
		if (ipz > 0)
		{
			zIn += ipz;
			countedIn++;
		}
	}
	float fact = tessCount.y;
	if (countedIn > 0)
		fact *= factor(zIn / countedIn);
	o.inside[0] = o.inside[1] = fact;

	float z[4];
	int counted[4];
	for (int i = 0; i < 4; i++)
	{
		z[i] = 0;
		counted[i] = 0;
	}

	for (int i = 0; i < INPUT_PATCH_EDGE; i++)
	{
		float ipz = ip[i * INPUT_PATCH_EDGE].cam.z;
		if (ipz > 0)
		{
			z[0] += ipz;
			counted[0]++;
		}

		ipz = ip[i].cam.z;
		if (ipz > 0)
		{
			z[1] += ipz;
			counted[1]++;
		}

		ipz = ip[i * INPUT_PATCH_EDGE + 3].cam.z;
		if (ipz > 0)
		{
			z[2] += ipz;
			counted[2]++;
		}

		ipz = ip[i + 12].cam.z;
		if (ipz > 0)
		{
			z[3] += ipz;
			counted[3]++;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		fact = tessCount.x;
		if (counted[i] > 0)
			fact *= factor(z[i] / counted[i]);
		o.edges[i] = fact;
	}
	return o;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(OUTPUT_PATCH_SIZE)]
[patchconstantfunc("HS_Patch")]
DSControlPoint main(InputPatch<HSInput, INPUT_PATCH_SIZE> ip, uint i : SV_OutputControlPointID,
					uint patchID : SV_PrimitiveID)
{
	DSControlPoint o;
	o.pos = ip[i].pos;
	return o;
}