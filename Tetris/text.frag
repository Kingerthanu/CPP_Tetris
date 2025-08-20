#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D fontTexture;

void main() {
    float alpha = texture(fontTexture, TexCoord).r;
    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}