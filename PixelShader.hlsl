//External texture-related data
Texture2D textureSRV		: register(t0);
TextureCube Sky				: register(t2);
SamplerState basicSampler	: register(s0);

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 normal       : NORMAL;       // Normal co-ordinates
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float2 uv           : TEXCOORD;     // UV co-ordinates
};


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
	input.tangent = normalize(input.tangent);
	
	float3 dirToLight1 = normalize(-light1.direction);
	float lightAmount1 = saturate(dot(input.normal, dirToLight1));

	float3 dirToLight2 = normalize(-light2.direction);
	float lightAmount2 = saturate(dot(input.normal, dirToLight2));

	float4 totalLight = ((light1.diffuseColor * lightAmount1 * surfaceColor) + light1.ambientColor * surfaceColor) + ((light2.diffuseColor * lightAmount2 * surfaceColor) + light2.ambientColor * surfaceColor);;
	return totalLight;
	
}