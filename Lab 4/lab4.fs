#version 410 core

out vec4 out_Color;
in vec2 gsTexCoord;
in vec3 gsNormal;
in float steep;
in float height;

uniform sampler2D grass_tex;
uniform sampler2D rock_tex;

vec4 rock(vec2 st){
    return texture(rock_tex, st);
}

vec4 grass(vec2 st){
    return texture(grass_tex, st);
}

void main(void)
{
    float frequency = 5.0;

    vec4 color1 = grass(gsTexCoord*frequency);
    vec4 color2 = rock(gsTexCoord*frequency);
    float ss = smoothstep(1.0, 1.1, height);
    //ss = step(1.0, height);
    vec4 color = mix(color1, color2, ss);
	float shade = normalize(gsNormal).z; // Fake light
//	out_Color = vec4(gsTexCoord.s, gsTexCoord.t, 0.0, 1.0);
	out_Color = vec4(gsNormal.x, gsNormal.y, gsNormal.z, 1.0);
    //out_Color = color; //* shade;
	//out_Color = vec4(shade, shade, shade, 1.0);
}

