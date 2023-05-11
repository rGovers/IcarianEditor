#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

layout(location = 0) uniform mat4 Transform;

layout(location = 0) out vec4 vColor;

void main()
{
    vColor = color;

    gl_Position = Transform * position;
}