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

cbuffer uniformBlock : register(b0, UPDATE_FREQ_PER_FRAME)
{
    float4x4 world;
	float4x4 projectView;
};

VsOut main(VsIn input)
{
	VsOut output = (VsOut)0;
    float4x4 tempMat = mul(projectView, world);
    output.position = mul(tempMat, input.position);

	output.texcoord = input.texcoord;

	return output;
};