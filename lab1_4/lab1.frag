// Lab 1 fragment shader
// Output either the generated texture from CPU or generate a similar pattern.
// Functions for 2D gradient and cellular noise included.

#version 150

out vec4 out_Color;
in vec2 texCoord;
uniform sampler2D tex;

uniform int displayGPUversion;
uniform float ringDensity;

uniform float brick_width = 1.25;
uniform float brick_height = 0.5;

uniform vec3 brick_color = vec3(0.6, 0.25, 0.15);
uniform vec3 dark_color = vec3(0.4, 0.15, 0.05);
uniform vec3 mortar_color = vec3(1.0, 0.8, 0.8);
uniform float mortar_width = 0.06;

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

vec2 tile(vec2 st, float zoom){
    st *= zoom;

    st.x += step(1., mod(st.y,2.0)) * 0.5;

    return fract(st);
}

vec3 uneven(vec2 st, vec2 uv, float rough){
    vec2 size = vec2(rough) * 1.0;
    return vec3(0, 0, 0);
}

void main(void)
{
	if (displayGPUversion == 1 || true)
	{
	    vec2 st = texCoord;
	    st /= vec2(brick_width, brick_height);
	    st = tile(st, 5.0);
	    vec3 color;
	    bool brick;

	    float n1 = noise(texCoord.yx*64.0);
	    st.x += n1*0.02;
	    st.y += n1*0.02;

	    if(st.x >= 1.0-mortar_width || st.x <= mortar_width || st.y >= 1.0-mortar_width || st.y <= mortar_width){
            color = mortar_color;
            brick = false;
	    }
	    else{
            color = brick_color;
            brick = true;
	    }

	    if(brick){
            float n = noise(texCoord*64.0);
            if(n > 0.2){
                color *= 0.9;
            }
            else if(n > 0.05){
                color *= 0.8;
            }
	    }


	    out_Color = vec4(color, 1.0);
	}
	else
		out_Color = texture(tex, texCoord);
}
