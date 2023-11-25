struct GSVertexUprocessed
{
    float4 STQ : TEXCOORD;
    uint4 RGBA : COLOR0;
    uint4 XYZSkip : POSITION;
};

struct VertexOutput
{
    float4 position : POSITION;
    float4 color : COLOR0;
};

VertexOutput vs_main(GSVertexUprocessed input)
{
    VertexOutput output;

    // Pass through position without any transformation
    output.position = float4(input.XYZSkip[0] * input.STQ.x, input.XYZSkip[1], input.XYZSkip[2], input.XYZSkip[3]);

    // Pass through color
    output.color = float4(input.RGBA[0], input.RGBA[1], input.RGBA[2], input.RGBA[3]);

    return output;
}

float4 ps_main(VertexOutput input) : SV_Target
{
    // Output the color without any modification
    return float4(input.color.r * input.position.x / 255.0, input.color.g / 255.0, input.color.b / 255.0, input.color.a / 255.0);
}