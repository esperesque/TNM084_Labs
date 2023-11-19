#version 150

in  vec3 inPosition;
in  vec3 inNormal;
in vec2 inTexCoord;

out vec3 exNormal;
out vec2 texCoord;
out vec3 exPos;
out float height;

uniform mat4 modelviewMatrix;
uniform mat4 projectionMatrix;

vec2 random2(vec2 st)
{
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
// This is a 2D gradient noise. Input your texture coordinates as argument, scaled properly.
float noise(vec2 st)
{
    vec2 i = floor(st);
    vec2 f = fract(st);

    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

void main(void)
{
	texCoord = inTexCoord;
	exNormal = inNormal;
	height = inPosition.y + 1.0;

	vec4 pos = vec4(inPosition, 1.0);
	if(pos.y < 0){
        pos.y = 0;
        //exNormal = vec3(0, 1, 0);
	}
	else{
        // tweak the height value randomly to create less straight seams between sand/grass
        // using the texture coordinates here did not work.
        height += noise(vec2(inPosition.x, inPosition.z)*0.3)*1.5;
    }

    exPos = inPosition;
	gl_Position = projectionMatrix * modelviewMatrix * pos;
}
