#define OUTPUT_PATCH_SIZE 16
#define N 4

static const float heightMult = 3.f;

cbuffer cbView : register(b0) //Domain Shader constant buffer slot 0
{
    matrix viewMatrix;
};

cbuffer cbCamPos : register(b1) //Domain Shader constant buffer slot 1
{
    float4 camPos;
};

cbuffer cbProj : register(b2) //Domain Shader constant buffer slot 2
{
    matrix projMatrix;
};

cbuffer cbTex : register(b3) //Domain Shader constant buffer slot 3
{
    matrix texMatrix;
};

Texture2D heightMap : register(t0);
SamplerState samp : register(s0);

struct HSPatchOutput
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct DSControlPoint
{
    float4 pos : SV_POSITION;
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

float4 getBezierPoint(float t, float4 pts[N], int n)
{
    float4 arr[N][N];
    for (int j = 0; j < n; j++)
    {
        arr[0][j] = pts[j];
    }
    for (int i = 1; i < n; i++)
    {
        for (int j = 0; j < n - i; j++)
        {
            arr[i][j] = (1 - t) * arr[i - 1][j] + t * arr[i - 1][j + 1];
        }
    }
    return arr[n - 1][0];
}

float4 getPatchPoint(float2 uv, int sx, int sz,
                     const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> input,
                     out float3 normal, out float3 tangent, out float3 bitangent)
{
    float4 p[N], pp[N], q[N], dp[N], dpp[N], qdpx[N], qdpy[N];
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            p[j] = input[i * N + j].pos;
            pp[j] = input[j * N + i].pos;
            if (j > 0)
            {
                dp[j - 1] = 3 * (p[j] - p[j - 1]);
                dpp[j - 1] = 3 * (pp[j] - pp[j - 1]);
            }
        }
        q[i] = getBezierPoint(uv.x, p, 4);
        qdpx[i] = getBezierPoint(uv.x, dp, 3);
        qdpy[i] = getBezierPoint(uv.y, dpp, 3);
    }
    tangent = normalize(getBezierPoint(uv.y, qdpx, 4).xyz);
    bitangent = normalize(getBezierPoint(uv.x, qdpy, 4).xyz);
    normal = -cross(bitangent, tangent);
    return getBezierPoint(uv.y, q, 4);
}

uint mipLevel(float z)
{
    float fact = -16.f * log10(0.01f * z);
    return max(6 - log2(fact), 1);
}

[domain("quad")]
PSInput main(HSPatchOutput factors, float2 uv : SV_DomainLocation,
             const OutputPatch<DSControlPoint, OUTPUT_PATCH_SIZE> input)
{
    PSInput o;
    float4 pt = getPatchPoint(uv, 0, 0, input, o.norm, o.tangent, o.bitangent);
    o.tex = mul(texMatrix, pt).xz;
    float tmpZ = mul(viewMatrix, pt).z;
    float heightDiff = heightMult * (heightMap.SampleLevel(samp, o.tex, mipLevel(tmpZ)).r - 0.5f);
    pt += float4(heightDiff * o.norm, 0.f);
    o.world = pt.xyz;
    o.view = normalize(camPos.xyz - o.world);
    o.pos = mul(viewMatrix, pt);
    o.pos = mul(projMatrix, o.pos);
    return o;
}