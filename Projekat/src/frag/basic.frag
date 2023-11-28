#version 330 core

in vec2 chTex; //koordinate teksture
out vec4 outCol;

uniform sampler2D uTex; //teksturna jedinica

void main()
{
	outCol = texture(uTex, chTex); //boja na koordinatama chTex teksture vezane na teksturnoj jedinici uTex
	
}