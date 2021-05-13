struct VsIn
{
	float4 position : Position;
	float2 texcoord : Texcoord;
};

struct VsOut
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

cbuffer uRootConstants : register(b0)
{
    float4x4 world;
	float4x4 viewProj;
};

VsOut main(VsIn input)
{
	VsOut output = (VsOut)0;
	output.position = mul(mul(input.position, world), viewProj);
	output.texcoord = input.texcoord;

	return output;
};