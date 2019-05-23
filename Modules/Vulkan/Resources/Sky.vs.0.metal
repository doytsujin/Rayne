#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct type_vertexUniforms
{
    float4x4 modelViewMatrix;
    float4x4 projectionMatrix;
};

struct sky_vertex_out
{
    float2 out_var_TEXCOORD0 [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct sky_vertex_in
{
    float3 in_var_POSITION [[attribute(0)]];
    float2 in_var_TEXCOORD0 [[attribute(5)]];
};

vertex sky_vertex_out sky_vertex(sky_vertex_in in [[stage_in]], constant type_vertexUniforms& vertexUniforms [[buffer(1)]])
{
    sky_vertex_out out = {};
    out.gl_Position = (vertexUniforms.projectionMatrix * float4((vertexUniforms.modelViewMatrix * float4(in.in_var_POSITION, 1.0)).xyz, 1.0)).xyww;
    out.out_var_TEXCOORD0 = in.in_var_TEXCOORD0;
    return out;
}
