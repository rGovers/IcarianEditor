#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec2 vUV;

void main()
{
    vec2 pos = vec2(gl_VertexID & 1, gl_VertexID >> 1);

    gl_Position = vec4(fma(pos.xy, vec2(2.0), vec2(-1.0)), 0.0, 1.0);
    vUV = pos;
}