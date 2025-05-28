R"(

#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in int i;

// texture sampler
uniform sampler2D texture1;
uniform sampler2D textures[2];

void main()
{
    vec4 color;
    // Workaround to deal with cannot index textures with non-constant
    if (i == 0) color = texture(textures[0], TexCoord);
    if (i == 1) color = texture(textures[1], TexCoord);
    // if (i == 0) color = red;
    // if (i == 1) color = green;
    FragColor = color;
}

)"
