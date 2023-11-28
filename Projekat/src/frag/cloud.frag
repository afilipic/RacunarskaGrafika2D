#version 330 core
out vec4 FragColor;

uniform vec4 cloudColor;

void main() {
    FragColor = cloudColor;
}