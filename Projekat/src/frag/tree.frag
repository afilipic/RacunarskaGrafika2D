#version 330 core
out vec4 FragColor;
uniform float paintedHeight;

void main()
{
    if (gl_FragCoord.y <= paintedHeight) {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // Bijela boja
    } else {
        FragColor = vec4(0.5, 0.25, 0.0, 1.0); // Originalna boja
    }
}