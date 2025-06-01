R"(

#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;
layout (location = 2) in float idx;

out vec2 TexCoord;
flat out int i;

void main()
{
    gl_Position = vec4(position, 0.0, 1.0);
    TexCoord = vec2(texcoord.x, texcoord.y);
    i = int(idx);
}

)"
