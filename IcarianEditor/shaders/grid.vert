// http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;

layout(location = 0) out vec3 near;
layout(location = 1) out vec3 far; 

void main()
{
    vec2 pos = vec2(gl_VertexID % 2, gl_VertexID / 2) * 2 - 1;
    mat4 vInv = inverse(view);
    mat4 pInv = inverse(proj);

    vec4 c = vInv * pInv * vec4(pos.xy, 0, 1);
    near = c.xyz / c.w;

    c = vInv * pInv * vec4(pos.xy, 1, 1);
    far = c.xyz / c.w;

    gl_Position = vec4(pos.xy, 0, 1);
}