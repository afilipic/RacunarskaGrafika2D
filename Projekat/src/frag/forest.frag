#version 330 core
out vec4 FragColor;

uniform vec2 v0;
uniform vec2 v1;
uniform vec2 v2;

void main()
{
    // Računanje barycentričkih koordinata
    vec2 v0v1 = v1 - v0;
    vec2 v0v2 = v2 - v0;
    vec2 p = gl_FragCoord.xy - v0;

    float dot00 = dot(v0v2, v0v2);
    float dot01 = dot(v0v2, v0v1);
    float dot02 = dot(v0v2, p);
    float dot11 = dot(v0v1, v0v1);
    float dot12 = dot(v0v1, p);

    float invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
    float w0 = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float w1 = (dot00 * dot12 - dot01 * dot02) * invDenom;
    float w2 = 1.0 - w0 - w1;

    // Faktor interpolacije na osnovu barycentričkih koordinata
    float t = clamp((p.x * w1 + v0.x * w2) / (v1.x * w1 + v0.x * w2), 0.0, 1.0);

    // Tamnozeleni gradijent za trougao (početna boja gradijenta)
    vec3 darkGreen = vec3(0.0, 0.05, 0.0);

    // Svetlozeleni gradijent za trougao (krajnja boja gradijenta)
    vec3 lightGreen = vec3(0.2, 0.7, 0.1);

    // Promenjen faktor boje za postizanje većeg učinka tamnije zelene
    vec3 interpolatedColor = mix(darkGreen, lightGreen, t);

    FragColor = vec4(interpolatedColor, 1.0); // Gradijent od tamnozelene do svetlozelene unutar trougla
}