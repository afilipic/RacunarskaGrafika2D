#version 330 core
    out vec4 FragColor;
    uniform float time; // Vreme za animaciju

    void main()
    {
        // Postepena promena boje između žute i narandžaste
        float mixFactor = (sin(time) + 1.0) / 2.0; // Normalizovano između 0.0 i 1.0
        vec3 yellowColor = vec3(1.0, 1.0, 0.0);
        vec3 orangeColor = vec3(1.0, 0.5, 0.0);
        vec3 color = mix(yellowColor, orangeColor, mixFactor);
        FragColor = vec4(color, 1.0);
    }