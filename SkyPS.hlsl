TextureCube Sky			: register(t0);
SamplerState Sampler	: register(s0);

// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvworld			: TEXCOORD;
};

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return Sky.Sample(Sampler, input.uvworld);
}