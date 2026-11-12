struct VSInput {
  float2 inPosition;
  float3 inColor;
}

// Buffer for model-view-projection matrix
struct UniformBuffer {
  float4x4 model;
  float4x4 view;
  float4x4 projection;
};
ConstantBuffer<UniformBuffer> ubo;

struct VSOutput {
  float4 pos : SV_Position;
  float3 color;
}

[shader("vertex")]
VSOutput VertexMain(VSInput input) 
{
  VSOutput output;
  /*
    Apply model-view-projection transformation to the vertex position
    Order of matrix multiplication is important:
    Model -> View -> Projection
    Model matrix: transforms the vertex from local object space to world space.
    View matrix: transforms the vertex from world space to camera (view) space.
    Projection matrix: transforms the vertex from camera space to clip space, applying perspective or orthographic projection.
  */
  output.pos = mul(ubo.projection, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 0.0, 1.0))));
  output.color = input.inColor;
  return output;
}

[shader("fragment")]
float4 FragmentMain(VSOutput vertIn) : SV_Target 
{
  return float4(vertIn.color, 1.0);
}