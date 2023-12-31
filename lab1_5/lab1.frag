// Lab 1 fragment shader
// Output either the generated texture from CPU or generate a similar pattern.
// Functions for 2D gradient and cellular noise included.

#version 150

out vec4 out_Color;
in vec2 texCoord;
uniform sampler2D tex;

uniform int displayGPUversion;
uniform float ringDensity;
uniform int time_ms; // tracks the time elapsed in MS

const float PI = 3.14; // I forget the other digits
//const float ringDensity = 10.0;

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

// Voronoise Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// http://iquilezles.org/www/articles/voronoise/voronoise.htm
// This is a variant of Voronoi noise.
// Usage: Call iqnoise() with the texture coordinates (typically scaled) as x, 1 to u (variation)
// and 0 to v (smoothing) for a typical Voronoi noise.
vec3 hash3( vec2 p )
{
    vec3 q = vec3( dot(p,vec2(127.1,311.7)),
                   dot(p,vec2(269.5,183.3)),
                   dot(p,vec2(419.2,371.9)) );
    return fract(sin(q)*43758.5453);
}

float iqnoise( in vec2 x, float u, float v )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    float k = 1.0+63.0*pow(1.0-v,4.0);

    float va = 0.0;
    float wt = 0.0;
    for (int j=-2; j<=2; j++)
	{
        for (int i=-2; i<=2; i++)
		{
            vec2 g = vec2(float(i),float(j));
            vec3 o = hash3(p + g)*vec3(u,u,1.0);
            vec2 r = g - f + o.xy;
            float d = dot(r,r);
            float ww = pow( 1.0-smoothstep(0.0,1.414,sqrt(d)), k );
            va += o.z*ww;
            wt += ww;
        }
    }

    return va/wt;
}

vec2 rotate2D(vec2 _st, float _angle){
    _st -= 0.5;
    _st =  mat2(cos(_angle),-sin(_angle),
                sin(_angle),cos(_angle)) * _st;
    _st += 0.5;
    return _st;
}

vec2 tile(vec2 st, float zoom){
    st *= zoom;
    return fract(st);
}

float box(vec2 st){
    if(st.x > 0.9 || st.x < 0.1 || st.y > 0.9 || st.x < 0.1){
        return 1.0;
    }
    else{
        return 0.0;
    }
}

float lines(vec2 st){
    st*= 4.0;
    st = tile(st, 2.0);
    float dist = sqrt(st.x*st.x + st.y*st.y);
    if((st.x > 0.9 || st.x < 0.1) && (dist > 0.4 && dist < 0.6)){
        return 1.0;
    }
    else{
        return 0.0;
    }
}

void main(void)
{
	if (displayGPUversion != 1)
	{
	    float time = time_ms / 1000.0; // time elapsed in seconds

	    float r = sin(noise(texCoord*1.2)*50+time);
	    float g = sin(noise(texCoord.yx)*20+time*0.7);
	    float b = sin(noise(texCoord*vec2(3.1, 8.5))*45+time*0.95);

	    out_Color = vec4(r*0.8, g*0.5, b, 1.0);

	    /*
		vec2 st = texCoord;
		vec2 st2 = texCoord;
		vec2 st3 = texCoord;
		st = rotate2D(st,-0.6*time);
		st = tile(st, time*0.7);

        st2 = rotate2D(st2,0.25*time);
		st2 = tile(st2, time*1.12);

		st3 = rotate2D(st3, 0.41*time);
		st3 = tile(st3, time*0.16);

		float st_box = box(st);
		float st2_box = box(st2);
		float st3_lines = lines(st3);

		vec3 color1 = vec3(st_box, 0.0, 0.0);
		vec3 color2 = vec3(0.0, st2_box, 0.0);
		vec3 color3 = vec3(st3_lines*0.5, st3_lines*0.5, st3_lines);
		vec3 color = mix(color1, color2, 0.5);
		color = mix(color, color3, 0.5);
		//vec3 color = color1;
		out_Color = vec4(color, 1.0);
		*/
	}
	else
		out_Color = texture(tex, texCoord);
}
