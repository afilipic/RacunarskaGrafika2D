#version 330 core
out vec4 FragColor;

void main()
{
    // Podela ekrana na dva dela - zemlja (dole) i nebo (gore)
    if (gl_FragCoord.y < 540.0)
    {
        // Tamnozelena boja za travu (prirodna nijansa)
        vec3 darkGreen = vec3(0.0, 0.25, 0.05);
        
        // Računanje udaljenosti od središta ekrana
        float distance = length(vec2(gl_FragCoord.x - 960.0, gl_FragCoord.y - 540.0));

        // Normalizacija udaljenosti na raspon [0, 1]
        float t = clamp(distance / 540.0, 0.0, 1.0);

        // Tamnija svetlozelena boja
        vec3 darkerLightGreen = vec3(0.2, 0.7, 0.1);

        // Interpolacija boje od tamnozelene do tamnije svetlozelene
        vec3 interpolatedColor = mix(darkGreen, darkerLightGreen, t);

        FragColor = vec4(interpolatedColor, 1.0); // Boja za travu kao gradijent
    }
    else
    {
        // Svetloplava boja za nebo
        FragColor = vec4(0.6, 0.8, 1.0, 1.0);
    }
}