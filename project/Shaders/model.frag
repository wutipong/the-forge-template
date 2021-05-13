struct PsIn
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

Texture2D uTex : register(t1);
SamplerState uSampler : register(s2);

float4 main(PsIn input) : SV_TARGET0
{
	return uTex.Sample(uSampler, input.texcoord);
};