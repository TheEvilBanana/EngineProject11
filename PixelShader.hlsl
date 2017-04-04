
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
};

Texture2D textureSRV : register(t0);
SamplerState basicSampler : register(s0);

struct DirectionalLight {
	float4 ambientColor;
	float4 diffuseColor;
	float3 direction;

};
cbuffer ExternalData : register(b0) {
	DirectionalLight light1;
	DirectionalLight light2;
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
	float4 surfaceColor = textureSRV.Sample(basicSampler, input.uv);

	input.normal = normalize(input.normal);
	
	float3 dirToLight1 = normalize(-light1.direction);
	float lightAmount1 = saturate(dot(input.normal, dirToLight1));

	float3 dirToLight2 = normalize(-light2.direction);
	float lightAmount2 = saturate(dot(input.normal, dirToLight2));

	float4 totalLight = ((light1.diffuseColor * lightAmount1 * surfaceColor) + light1.ambientColor * surfaceColor) + ((light2.diffuseColor * lightAmount2 * surfaceColor) + light2.ambientColor * surfaceColor);;
	return totalLight;
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}