 #version 330 core
    layout (location = 0) in vec2 aPos;
    uniform float sizeFactor; // Faktor veličine za Sunce
    uniform vec2 center; // Centar Sunca

    void main()
    {
        // Pomeranje verteksa tako da je centar Sunca u koordinatnom početku
        vec2 pos = aPos - center; 

        // Skaliranje verteksa
        pos *= sizeFactor;

        // Pomeranje verteksa nazad
        pos += center;

        gl_Position = vec4(pos, 0.0, 1.0);
    }