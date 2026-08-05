// Sample Vertex Shader
// This shader is a simple example of a vertex shader that outputs the 
// positions of three vertices to form a triangle.

// Hardcode vertex positions to form triangle.
static float2 positions[3] = float2[](
  float2(0.0, -0.5),
  float2(0.5, 0.5),
  float2(-0.5, 0.5)
);

// List of RGB colors for the vertices
static float3 colors[3] = float3[](
  float3(1.0, 0.0, 0.0), // Red
  float3(0.0, 1.0, 0.0), // Green
  float3(0.0, 0.0, 1.0)  // Blue
);

struct VertexOutput 
{
  float3 color;
  float4 sv_position : SV_Position;
};

// Vertex Shader
[shader("vertex")]
VertexOutput VertexMain(uint vertexId : SV_VertexID)
{
  VertexOutput output;
  output.sv_position = float4(positions[vertexId], 0.0, 1.0);
  output.color = colors[vertexId];
  return output;
}

// Fragment Shader
[shader("fragment")]
float4 FragmentMain(VertexOutput inVert) : SV_Target
{
  return float4(inVert.color, 1.0); // Output red color
}