#version 410 core

layout(triangles) in;
// Use line_strip for visualization and triangle_strip for solids
layout(triangle_strip, max_vertices = 3) out;
//layout(line_strip, max_vertices = 3) out;
in vec2 teTexCoord[3];
in vec3 teNormal[3];
out vec2 gsTexCoord;
out vec3 gsNormal;
uniform sampler2D tex;

uniform mat4 projMatrix;
uniform mat4 mdlMatrix;
uniform mat4 camMatrix;

uniform float disp;
uniform int texon;

vec2 random2(vec2 st)
{
    st = vec2( dot(st,vec2(127.1,311.7)),
              dot(st,vec2(269.5,183.3)) );
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
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

vec3 random3(vec3 st)
{
    st = vec3( dot(st,vec3(127.1,311.7, 543.21)),
              dot(st,vec3(269.5,183.3, 355.23)),
              dot(st,vec3(846.34,364.45, 123.65)) ); // Haphazard additional numbers by IR
    return -1.0 + 2.0*fract(sin(st)*43758.5453123);
}

// Gradient Noise by Inigo Quilez - iq/2013
// https://www.shadertoy.com/view/XdXGW8
// Trivially extended to 3D by Ingemar
float noise(vec3 st)
{
    vec3 i = floor(st);
    vec3 f = fract(st);

    vec3 u = f*f*(3.0-2.0*f);

    return mix(
    			mix( mix( dot( random3(i + vec3(0.0,0.0,0.0) ), f - vec3(0.0,0.0,0.0) ),
                     dot( random3(i + vec3(1.0,0.0,0.0) ), f - vec3(1.0,0.0,0.0) ), u.x),
                mix( dot( random3(i + vec3(0.0,1.0,0.0) ), f - vec3(0.0,1.0,0.0) ),
                     dot( random3(i + vec3(1.0,1.0,0.0) ), f - vec3(1.0,1.0,0.0) ), u.x), u.y),

    			mix( mix( dot( random3(i + vec3(0.0,0.0,1.0) ), f - vec3(0.0,0.0,1.0) ),
                     dot( random3(i + vec3(1.0,0.0,1.0) ), f - vec3(1.0,0.0,1.0) ), u.x),
                mix( dot( random3(i + vec3(0.0,1.0,1.0) ), f - vec3(0.0,1.0,1.0) ),
                     dot( random3(i + vec3(1.0,1.0,1.0) ), f - vec3(1.0,1.0,1.0) ), u.x), u.y), u.z

          	);
}

// rotation around arbitrary axis
vec3 rot(vec3 v, vec3 axis, float angle){
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float ic = 1.0 - c;

    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    mat3 rm = mat3(c+x*x*ic, y*x*ic+z*s, x*z*ic-y*s, x*y*ic-z*s, c+y*y*ic, z*y*ic+x*s, x*z*ic+y*s, y*z*ic-x*s, c+z*z*ic);
    return rm*v;
}

float fbm(vec3 st){
    int octaves = 3;
    float amplitude = 0.2;
    float lacunarity = 2.0;
    float frequency = 3.0;
    float persistence = 5.0;
    float gain = pow(2.0, -persistence);

    float val = 0.0;

    for(int i = 0; i < octaves; i++){
        vec3 off = vec3(1.74, 2.25, 3.42);
        float gn = noise(st*frequency + off*i);
        val += gn * amplitude;

        // Update values for the next pass
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return val;
}

void computeVertex(int nr)
{
	vec3 p, v1, v2, v3, p1, p2, p3, s1, s2, n;

	p = vec3(gl_in[nr].gl_Position);

	// Calculate normals
	vec3 n1 = normalize(p); // This is the position-based normal
	vec3 sf; // Vector parallel to surface
	// Given a position and a normal, the equation for the plane is a(x-x1)+b(y-y1)+c(z-z1) = 0
	// Setting x = 0 and y = 0 gives n1.x(1-p.x)+n1.y(1-p.y)+n1.z(z-p.z) = 0
	// nl.z(z-p.z) = -nl.x(1-p.x)-nl.y(1-p.y)
	// z-p.z = (-nl.x(1-p.x)-nl.y(1-p.y)) / nl.z <=> z = ((-nl.x(-p.x)-nl.y(-p.y)) / nl.z) + p.z - will not work if nl.z = 0
	if(n1.x != 0){
        sf = vec3(0, 0, 0);
        sf.x = ((n1.z*p.z+n1.y*p.y) / n1.x) + p.x;
	}
	else if(n1.y != 0){
	    sf = vec3(0, 0, 0);
        sf.y = ((n1.z*p.z+n1.x*p.x) / n1.y) + p.y;
	}
	else{
        sf = vec3(0, 0, 0);
        sf.z = ((n1.y*p.y+n1.x*p.x) / n1.z) + p.z;
	}

	//sf = sf - p;
    sf = normalize(sf);
    float surface_offset = 0.1;
    sf *= surface_offset;
    p1 = sf;
    p2 = rot(sf, n1, (2.0/3.0)*3.14);
    p3 = rot(sf, n1, (4.0/3.0)*3.14);

    p1 *= (1.0 + fbm(p1));
    p2 *= (1.0 + fbm(p2));
    p3 *= (1.0 + fbm(p3));

    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal

    vec3 u = p2 - p1;
    vec3 v = p3 - p1;
    n = vec3(u.y*v.z - u.z*v.y, u.z*v.x-u.x*v.z, u.x*v.y-u.y*v.x);

	// Add interesting code here
	p = normalize(p);

	p *= (1.0 + fbm(p));

	gl_Position = projMatrix * camMatrix * mdlMatrix * vec4(p, 1.0);

    gsTexCoord = teTexCoord[0];

	//n = teNormal[nr]; // This is not the normal you are looking for. Move along!
    gsNormal = mat3(camMatrix * mdlMatrix) * n;
    EmitVertex();
}

void main()
{
	computeVertex(0);
	computeVertex(1);
	computeVertex(2);
}

