Texture2D Texture;
SamplerState Sampler;
cbuffer cbuf
{
	uint2 divisions;
	uint2 unused;
	float4 brightness;
};

float4 Process(float2 tc : TextureCoordinate) : SV_TARGET
{
	uint2 cell = uint2(uint(tc.x * float(divisions.x)), uint(tc.y * float(divisions.y)));
	float2 newtc = float2(float(cell.x) / float(divisions.x), float(cell.y) / float(divisions.y));
	return Texture.Sample(Sampler, newtc) * brightness;
}