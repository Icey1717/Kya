#define int12_to_float(x)	(float)((float)x * 0.000244140625f)

cbuffer UniformBuffer : register(b0)
{
    float4x4 model;
    float4x4 view;
    float4x4 proj;
    float4x4 animMatrices[0x18]; // Projection * View matrix
};

struct GSVertexUprocessed
{
    float4 STQ : TEXCOORD;
    uint4 RGBA : COLOR0;
    float4 XYZSkip : POSITION;
};

struct VertexOutput
{
    float4 position : POSITION;
    float4 color : COLOR0;
};

VertexOutput vs_main(GSVertexUprocessed input, uint vertexID : SV_VertexID)
{
    VertexOutput output;
    
    // _$XYZW_16_Conv_EndBones_Rigid
    uint flagsAnim = asuint(input.XYZSkip.w);
    
    uint anim = flagsAnim & 0x7ff;
    uint flags = flagsAnim & 0xc000;
    
    input.XYZSkip.w = flags;
    
    uint animIndex = anim - 0x3dc;
    animIndex = animIndex / 4;
    
    float4 animated = mul(animMatrices[animIndex], float4(input.XYZSkip.xyz, 1.0f));
    input.XYZSkip.xyz = animated.xyz;
    
    float S = int12_to_float(asint(input.STQ.x));
    input.STQ.x = S;
    
    float T = int12_to_float(asint(input.STQ.y));
    input.STQ.y = T;
    
    float4 color = float4((float) input.RGBA.x, (float) input.RGBA.y, (float) input.RGBA.z, (float) input.RGBA.w);
    
    // _$Lightning_Repack
    input.RGBA.xyz = float3((int) color.x, (int) color.y, (int) color.z);
    
    // _$GouraudMapping_No_Fog_16_2()
#if 0
    float4 screenSpace = mul(objToScreen, float4(input.XYZSkip.xyz, 1.0f));
    
    float Q = 1.0f / screenSpace.w;
    
    input.STQ.xyz = input.STQ.xyz * Q;
    screenSpace.xyz = screenSpace.xyz * Q;

    // Pass through position without any transformation
    output.position = float4(screenSpace.xyz, input.XYZSkip.w);
#endif
    
    float4 fixedPos = float4(input.XYZSkip.xyz, 1.0);
    output.position = mul(proj, mul(view, mul(model, fixedPos)));
    output.color = float4(input.RGBA[0] / 255.0, input.RGBA[1] / 255.0, input.RGBA[2] / 255.0, input.RGBA[3] / 255.0);
    return output;
}

struct PSOutput
{
    float4 outColor : SV_TARGET;
};

PSOutput ps_main(VertexOutput input)
{
    PSOutput output;
    output.outColor = input.color * input.position.x;
    return output;
}