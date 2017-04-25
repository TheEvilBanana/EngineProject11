// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	//float4 color		: COLOR;        // RGBA color
	float3 normal       : NORMAL;       // Normal co-ordinates
	float2 uv           : TEXCOORD;     // UV co-ordinates
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
};

Texture2D textureSRV : register(t0);
Texture2D normalMapSRV : register(t1);

SamplerState basicSampler : register(s0);

struct DirectionalLight {
	float4 ambientColor;
	float4 diffuseColor;
	float3 direction;

};
cbuffer ExternalData : register(b0) {
	DirectionalLight dirLight1;
	//DirectionalLight dirLight2;
};

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{

	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	float3 normalFromMap = normalMapSRV.Sample(basicSampler, input.uv).xyz * 2 - 1;

	// Transform from tangent to world space
	float3 N = input.normal;
	float3 T = normalize(input.tangent - N * dot(input.tangent, N));
	float3 B = cross(T, N);

	float3x3 TBN = float3x3(T, B, N);
	input.normal = normalize(mul(normalFromMap, TBN));

	float lightAmount1 = saturate(dot(input.normal, -normalize(dirLight1.direction)));

	//float lightAmount2 = saturate(dot(input.normal, -normalize(dirLight2.direction)));

	float4 surfaceColor = textureSRV.Sample(basicSampler, input.uv);

	float4 light1 = ((dirLight1.diffuseColor * lightAmount1 * surfaceColor) + (dirLight1.ambientColor * surfaceColor));
	//float4 light2 = ((dirLight2.diffuseColor * lightAmount2 * surfaceColor) + (dirLight2.ambientColor * surfaceColor));
	//float4 totalLight = light1 + light2;

	return light1;
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}