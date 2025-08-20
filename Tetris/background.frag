#version 330 core
out vec4 FragColor;

in vec3 vertColor;
uniform vec3 bgColor;

void main() 
{

    FragColor = vec4(vertColor + bgColor, 1.0); // Pink/red color

}