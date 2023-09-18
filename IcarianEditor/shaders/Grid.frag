// http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
#version 450

#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) uniform mat4 view;
layout(location = 1) uniform mat4 proj;

layout(location = 0) out vec4 color;

layout(location = 0) in vec3 near;
layout(location = 1) in vec3 far; 

void main() 
{
    float t = -near.y / (far.y - near.y);

    if (t <= 0)
    {
        discard;
    }

    vec3 fragPos = near + t * (far - near);

    vec2 der = fwidth(fragPos.xz);
    vec2 grid = abs(fract(fragPos.xz - 0.5.xx) - 0.5.xx) / der;
    float l = min(grid.x, grid.y);

    vec4 col = vec4(0.4.xxx, 1 - min(l, 1));
    
    if(fragPos.z > -0.025 && fragPos.z < 0.025)
    {
        col = vec4(1, 0, 0, 1);
    }
    
    if(fragPos.x > -0.025 && fragPos.x < 0.025)
    {
        col = vec4(0, 0, 1, 1);
    }

    vec4 cPos = proj * view * vec4(fragPos, 1);
    float v = cPos.z / cPos.w;

    gl_FragDepth = v * 0.5f + 0.5f;
    // gl_FragDepth = v;

    float oV = v * 2 - 1;
    float lD = (2 * 0.1 * 100) / (100 + 0.1 - oV * (100 - 0.1));

    float sD = lD / 100;

    col.a *= 1 - sD;

    color = col;
}