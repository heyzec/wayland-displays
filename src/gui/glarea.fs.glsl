R"(

#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
flat in int i;

// MAX_DISPLAYS is 16;

uniform sampler2D textures[16];

void main()
{
    vec4 color;
    // Workaround for the common "Y-flipping issue" caused by coordinate origin discrepancy in OpenGL vs image formats
    vec2 coord = vec2(TexCoord.x, 1 - TexCoord.y);
    // Workaround to deal with cannot index textures with non-constant
    // Set up to MAX_DISPLAYS
    if (i == 0) color = texture(textures[0], coord);
    if (i == 1) color = texture(textures[1], coord);
    if (i == 2) color = texture(textures[2], coord);
    if (i == 3) color = texture(textures[3], coord);
    if (i == 4) color = texture(textures[4], coord);
    if (i == 5) color = texture(textures[5], coord);
    if (i == 6) color = texture(textures[6], coord);
    if (i == 7) color = texture(textures[7], coord);
    if (i == 8) color = texture(textures[8], coord);
    if (i == 9) color = texture(textures[9], coord);
    if (i == 10) color = texture(textures[10], coord);
    if (i == 11) color = texture(textures[11], coord);
    if (i == 12) color = texture(textures[12], coord);
    if (i == 13) color = texture(textures[13], coord);
    if (i == 14) color = texture(textures[14], coord);
    if (i == 15) color = texture(textures[15], coord);
    FragColor = color;
}

)"
