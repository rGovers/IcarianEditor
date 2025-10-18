#version 450

layout(location = 0) uniform sampler2D samplerA;
layout(location = 1) uniform sampler2D samplerB;

layout(location = 0) in vec2 vUV;

layout(location = 0) out vec4 color;

void main()
{
    vec4 colorA = texture(samplerA, vUV);
    vec4 colorB = texture(samplerB, vUV);

    color = vec4(colorA.xyz * colorA.a + colorB.xyz * colorB.a, 1.0);
}