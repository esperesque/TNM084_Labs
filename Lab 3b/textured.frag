#version 150

out vec4 outColor;

in vec2 texCoord;
in vec3 exNormal;
in float height;
in vec3 exPos;
uniform sampler2D tex;
uniform sampler2D rock_tex;
uniform int time_ms; // tracks the time elapsed in MS
uniform vec3 sun_pos;
uniform float sun_x;
uniform float sun_y;
uniform float sun_z;

const vec3 SAND_CLR = vec3(1.0, 0.8, 0.4);
const vec3 LIGHT_SOURCE = vec3(10000.0, 10000.0, 0.0);

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

vec3 rock(vec2 st){
    vec3 grass_color = texture(rock_tex, st*0.2).xyz;
    return grass_color;
    //return texture(rock_tex, st*0.2).xyz;
}

vec3 grass(vec2 st, vec3 norm){
    vec3 grass_color = texture(tex, st*0.2).xyz;

    float r = dot(norm, vec3(0,1.0,0));
    r = smoothstep(0.85, 0.95, r); // shows rock if r is [0, 0.5]

    return(mix(rock(st), grass_color, r));

    //return grass_color;
}

vec3 sand(vec2 st){
    vec3 sand_color = SAND_CLR + abs(noise(st*5)) * SAND_CLR * 0.3;
    // solid base color, try adding grains
    //float grain = abs(random2(st.yx).x);
    float grain = abs(noise(st.yx * 50));
    sand_color -= step(grain, 0.03) * vec3(1.0, 0.8, 0.0) * 0.1;
    return sand_color;
}

vec3 ripple(vec3 color, vec2 st){
    st.x += 0.05*sin(time_ms*0.008);
    st.y += time_ms*0.0001;
    vec3 clr = sand(st);
    vec3 clr2 = sand(st.yx*1.5);
    clr = mix(clr, clr2, 0.5+0.3*sin(time_ms*0.008));
    float visible = smoothstep(-20.0, 0.0, height);
    visible = pow(visible, 30);
    clr = mix(vec3(0, 0, 1.0), clr, visible-0.1);
    return clr;
}

void main(void)
{
	// Texture from disc
	// t = texture(tex, texCoord);

	vec3 n = normalize(exNormal);
	float shade;
	// Reflectivity of the surface material
	if(height <= 0){
        shade = 1.0;
	}
	else{
        // calculate lighting
        vec3 sun = vec3(sun_x, sun_y, sun_z);
        vec3 light_dir = sun - exPos;
        light_dir = normalize(light_dir);
        float light = dot(light_dir, n);

        shade = 0.5 + 0.5*light;
        //shade = 0.5 + 0.5*min((n.y + n.z), 1.0);
        shade += (1.0 - smoothstep(0.0, 3.0, height));
        shade = min(shade, 1.0);
	}

	vec3 color;

	// Procedural texture

    float sg = smoothstep(1.0, 1.5, height);
    vec3 color1 = sand(texCoord);
    vec3 color2 = grass(texCoord, n);
    color = mix(color1, color2, sg);

    float wl = smoothstep(-0.1, 0.1, height);
    color = mix(color, ripple(color, texCoord), (1.0-wl));

	//color = t.xyz;
	//shade = 1;
    outColor = vec4(color * shade * shade, 1.0); // Over-emphasized fake light

//	outColor = vec4(texCoord.s, texCoord.t, 0, 1);
//	outColor = vec4(n.x, n.y, n.z, 1);
//	outColor = vec4(1) * shade;
}
