 #version 330 core
    out vec4 FragColor;
    
    uniform vec3 circleColor; // Boja kruga

    void main()
    {
        vec4 color = vec4(circleColor, 0.7); // Postavite alfa kanal na 0.5 za providnost
        FragColor = color;
    }