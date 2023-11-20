// Lab 3b, C++ version
// Terrain generation

// Current contents:
// Terrain being generated on CPU (MakeTerrain). Simple terrain as example: Flat surface with a bump.
// Note the constants kTerrainSize and kPolySize for changing the number of vertices and polygon size!

// Things to do:
// Generate a terrain on CPU, with normal vectors
// Generate a terrain on GPU, in the vertex shader, again with normal vectors.
// Generate textures for the surface (fragment shader).

// If you want to use gradient noise, use the code from Lab 1.

#define MAIN
#include <iostream>
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoaderX.h"
#include "LoadTGA.h"
#include <math.h>
#include "noise1234.h"
#include "simplexnoise1234.h"
#include "cellular.h"
// uses framework OpenGL
// uses framework Cocoa

mat4 projectionMatrix;
Model *floormodel;
GLuint grasstex;
GLuint rocktex;

// Reference to shader programs
GLuint phongShader, texShader;

#define kTerrainSize 512
#define kPolySize 0.5

// Terrain data. To be initialized in MakeTerrain or in the shader
vec3 vertices[kTerrainSize*kTerrainSize];
vec2 texCoords[kTerrainSize*kTerrainSize];
vec3 normals[kTerrainSize*kTerrainSize];
GLuint indices[(kTerrainSize-1)*(kTerrainSize-1)*3*2];

// These are considered unsafe, but with most C code, write with caution.
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

// Implementation of multi-octave Perlin partially from https://github.com/simondevyoutube/ProceduralTerrain_Part2/tree/master/src
float PerlinFBM(float x, float z){
    int octaves = 5;
    float scale = 1.0;
    float lacunarity = 2.0;
    float frequency = 0.02;
    float persistence = 2.0;
    float amplitude = 6.0;
    float gain = pow(2.0, -persistence);
    float norm_factor = 0;

    float xs = x * scale;
    float zs = z * scale;

    float val = 0.0;

    for(int i = 0; i < octaves; i++){
        float noise = snoise3(xs*frequency, zs*frequency, octaves);

        val += noise * amplitude;

        // Update values for the next pass
        norm_factor += amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }

    // normalize
    //val /= norm_factor;

    return val;
}

void MakeTerrain()
{
    //GLfloat heightmap[kTerrainSize*kTerrainSize];

	// TO DO: This is where your terrain generation goes if on CPU.
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
		int ix = z * kTerrainSize + x;

		float y = PerlinFBM(x, z);

		vertices[ix] = vec3(x * kPolySize, y, z * kPolySize);
		texCoords[ix] = vec2(x, z);
		normals[ix] = vec3(0,1,0);
		//heightmap[ix] = y;
	}

	// Make indices
	// You don't need to change this.
	for (int x = 0; x < kTerrainSize-1; x++)
	for (int z = 0; z < kTerrainSize-1; z++)
	{
		// Quad count
		int q = (z*(kTerrainSize-1)) + (x);
		// Polyon count = q * 2
		// Indices
		indices[q*2*3] = x + z * kTerrainSize; // top left
		indices[q*2*3+1] = x+1 + z * kTerrainSize;
		indices[q*2*3+2] = x + (z+1) * kTerrainSize;
		indices[q*2*3+3] = x+1 + z * kTerrainSize;
		indices[q*2*3+4] = x+1 + (z+1) * kTerrainSize;
		indices[q*2*3+5] = x + (z+1) * kTerrainSize;
	}

	// Make normal vectors
	// TO DO: This is where you calculate normal vectors
	for (int x = 0; x < kTerrainSize; x++)
	for (int z = 0; z < kTerrainSize; z++)
	{
	    vec3 L; // Left neighbor (negative x-axis)
	    vec3 R; // Right neighbor (positive x-axis)
	    vec3 F; // Front neighbor (negative z-axis)
	    vec3 B; // Back neighbor (positive z-axis)

	    // In case of edge/corner vertices, use the normal of the central vertex
	    // in place of the neighbor vertex that is out of bounds
	    // Equivalent to "replicate padding" in image processing

	    if(x == 0){
            L = vertices[z*kTerrainSize + x];
            R = vertices[z*kTerrainSize + x+1];
	    }
	    else if(x == kTerrainSize - 1){
            L = vertices[z*kTerrainSize + x-1];
            R = vertices[z*kTerrainSize + x];
	    }
	    else{
            L = vertices[z*kTerrainSize + x-1];
            R = vertices[z*kTerrainSize + x+1];
	    }

	    if(z == 0){
            F = vertices[z*kTerrainSize + x];
            B = vertices[(z+1)*kTerrainSize + x];
	    }
	    else if(z == kTerrainSize - 1){
            F = vertices[(z-1)*kTerrainSize + x];
            B = vertices[z*kTerrainSize + x];
	    }
	    else{
            F = vertices[(z-1)*kTerrainSize + x];
            B = vertices[(z+1)*kTerrainSize + x];
	    }

	    // Cross product method suggested in Le6
	    vec3 v1 = (R - L);
	    vec3 v2 = (B - F);
	    vec3 norm = normalize(cross(v2, v1));

        normals[z * kTerrainSize + x] = SetVec3(norm.x, norm.y, norm.z);
	}
}

void init(void)
{
	// GL inits
	glClearColor(0.4,0.5,0.8,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	projectionMatrix = frustum(-0.15 * 640/480, 0.1 * 640/480, -0.1, 0.1, 0.2, 300.0);

	// Load and compile shader
	phongShader = loadShaders("phong.vert", "phong.frag");
	texShader = loadShaders("textured.vert", "textured.frag");
	printError("init shader");

	// Upload geometry to the GPU:
	MakeTerrain();
	floormodel = LoadDataToModel(vertices, normals, texCoords, NULL,
			indices, kTerrainSize*kTerrainSize, (kTerrainSize-1)*(kTerrainSize-1)*2*3);

	printError("LoadDataToModel");

// Important! The shader we upload to must be active!
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(phongShader, "tex"), 0); // Texture unit 0
	glUniform1i(glGetUniformLocation(phongShader, "rock_tex"), 1); // Texture unit 1

	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(texShader, "tex"), 0); // Texture unit 0
	glUniform1i(glGetUniformLocation(texShader, "rock_tex"), 1); // Texture unit 0

	LoadTGATextureSimple("grass2.tga", &grasstex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	glActiveTexture(GL_TEXTURE1);
	LoadTGATextureSimple("rock4.tga", &rocktex);
	glBindTexture(GL_TEXTURE_2D, rocktex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	printError("init arrays");
}

vec3 campos = vec3(kTerrainSize*kPolySize/4, 1.5, kTerrainSize*kPolySize/4);
vec3 forward = vec3(8, 0, 8);
vec3 up = vec3(0, 1, 0);

vec3 y_rot(vec3 v, float th){
    mat3 rm;
    rm.m[0] = cos(th);    rm.m[1] = 0;      rm.m[2] = sin(th);
    rm.m[3] = 0;          rm.m[4] = 1;      rm.m[5] = 0;
    rm.m[6] = -sin(th);   rm.m[7] = 0;      rm.m[8] = cos(th);

    return MultMat3Vec3(rm, v);
}

// Called approximately 60 times per second
void animate(int t){
    //std::cout << t << "\n";
    glutTimerFunc(16, animate, t+16);
    glUniform1i(glGetUniformLocation(texShader, "time_ms"), t);

    //GLfloat sun_pos; // = GLfloat([10000.0, 10000.0, 0]);
    //sun_pos[0] = 10000.0;
    //sun_pos[1] = 10000.0;
    //sun_pos[2] = 0.0;

    vec3 sun_pos = vec3(10000.0, 10000.0, 0.0);
    float th = 0.001*t;
    sun_pos = y_rot(sun_pos, th);

    glUniform1f(glGetUniformLocation(texShader, "sun_x"), sun_pos.x);
    glUniform1f(glGetUniformLocation(texShader, "sun_y"), sun_pos.y);
    glUniform1f(glGetUniformLocation(texShader, "sun_z"), sun_pos.z);

    // This line should send a vec3 to the fragment shader. however, it does not work.
    //glUniform3fv(texShader, sun_pos.size(), reinterpret_cast<GLfloat *>(sun_pos.data()));
    //glUniform1fv(glGetUniformLocation(texShader, "sun_pos"), sun_pos);

    glutPostRedisplay();
}

void display(void)
{
	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 worldToView, m;

	if (glutKeyIsDown('a'))
		forward = mat3(Ry(0.03))* forward;
	if (glutKeyIsDown('d'))
		forward = mat3(Ry(-0.03)) * forward;
	if (glutKeyIsDown('w'))
		campos = campos + forward * 0.01;
	if (glutKeyIsDown('s'))
		campos = campos - forward * 0.01;
	if (glutKeyIsDown('q'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		campos = campos - side * 0.01;
	}
	if (glutKeyIsDown('e'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		campos = campos + side * 0.01;
	}

	// Move up/down
	if (glutKeyIsDown('z'))
		campos = campos + vec3(0,1,0) * 0.01;
	if (glutKeyIsDown('c'))
		campos = campos - vec3(0,1,0) * 0.01;

	// NOTE: Looking up and down is done by making a side vector and rotation around arbitrary axis!
	if (glutKeyIsDown('+'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		mat4 m = ArbRotate(side, 0.01);
		forward = mat3(m) * forward;
	}
	if (glutKeyIsDown('-'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		mat4 m = ArbRotate(side, -0.01);
		forward = m * forward;
	}

	worldToView = lookAtv(campos, campos + forward, up);

	glBindTexture(GL_TEXTURE_2D, grasstex); // The texture is not used but provided as example
	glBindTexture(GL_TEXTURE_2D, rocktex);
	// Floor
	GLuint shader = texShader;
	glUseProgram(shader);
	m = worldToView;
	glUniformMatrix4fv(glGetUniformLocation(shader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(floormodel, shader, "inPosition", "inNormal", "inTexCoord");

	printError("display");

	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitWindowSize(640,360);
	glutCreateWindow ("Lab 3b");
	glutRepeatingTimer(20);
	glutTimerFunc(16, animate, 0);
	glutDisplayFunc(display);
	glutKeyboardFunc(keys);
	init ();
	glutMainLoop();
}
