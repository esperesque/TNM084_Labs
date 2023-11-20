
// Fractal tree generation

// C++ version 2022.
// Adapted to lastest GLUGG.

#define MAIN
#include <iostream>
#include "MicroGlut.h"
#include "GL_utilities.h"
#include "VectorUtils4.h"
#include "LittleOBJLoaderX.h"
#include "LoadTGA.h"
#include "glugg.h"
#include "simplexnoise1234.h"

//#include <iostream>

// uses framework OpenGL
// uses framework Cocoa

void MakeCylinderAlt(int aSlices, float height, float topwidth, float bottomwidth)
{
	gluggMode(GLUGG_TRIANGLE_STRIP);
	vec3 top = SetVector(0,height,0);
	vec3 center = SetVector(0,0,0);
	vec3 bn = SetVector(0,-1,0); // Bottom normal
	vec3 tn = SetVector(0,1,0); // Top normal

	for (float a = 0.0; a < 2.0*M_PI+0.0001; a += 2.0*M_PI / aSlices)
	{
		float a1 = a;

		vec3 p1 = SetVector(topwidth * cos(a1), height, topwidth * sin(a1));
		vec3 p2 = SetVector(bottomwidth * cos(a1), 0, bottomwidth * sin(a1));
		vec3 pn = SetVector(cos(a1), 0, sin(a1));

// Done making points and normals. Now create polygons!
		gluggNormalv(pn);
	    gluggTexCoord(height, a1/M_PI);
	    gluggVertexv(p2);
	    gluggTexCoord(0, a1/M_PI);
	    gluggVertexv(p1);
	}

	// Then walk around the top and bottom with fans
	gluggMode(GLUGG_TRIANGLE_FAN);
	gluggNormalv(bn);
	gluggVertexv(center);
	// Walk around edge
	for (float a = 0.0; a <= 2.0*M_PI+0.001; a += 2.0*M_PI / aSlices)
	{
		vec3 p = vec3(bottomwidth * cos(a), 0, bottomwidth * sin(a));
	    gluggVertexv(p);
	}
	// Walk around edge
	gluggMode(GLUGG_TRIANGLE_FAN); // Reset to new fan
	gluggNormalv(tn);
	gluggVertexv(top);
	for (float a = 2.0*M_PI; a >= -0.001; a -= 2.0*M_PI / aSlices)
	{
		vec3 p = vec3(topwidth * cos(a), height, topwidth * sin(a));
	    gluggVertexv(p);
	}
}


mat4 projectionMatrix;

Model *floormodel;
GLuint grasstex, barktex;

// Reference to shader programs
GLuint phongShader, texShader;

// Floor quad
GLfloat vertices2[] = {	-20.5,0.0,-20.5,
						20.5,0.0,-20.5,
						20.5,0.0,20.5,
						-20.5,0.0,20.5};
GLfloat normals2[] = {	0,1.0,0,
						0,1.0,0,
						0,1.0,0,
						0,1.0,0};
GLfloat texcoord2[] = {	50.0f, 50.0f,
						0.0f, 50.0f,
						0.0f, 0.0f,
						50.0f, 0.0f};
GLuint indices2[] = {	0,3,2, 0,2,1};

// THIS IS WHERE YOUR WORK GOES!

// TREE PARAMETERS
const float BRANCH_SCALE = 0.75;
const int MAX_DEPTH = 9; // Crashes at depth = 10

long long seed = 127442751;

// Uses Blum Blum Shub to generate a random float in the range [0, 1]
float randf(){
    long m1 = 6229; // long is 32 bits
    long m2 = 7757;

    long long s = (seed*seed) % (m1*m2);
    seed = s;
    return((float)s / (m1*m2));
}

// Uses the randf function to generate a random angle between 0 and 2*pi radians
float rand_ang(){
    float pi = 3.14; // TODO: look up the other digits
    return(pi * 2 * randf());
}

float rand_range(float a, float b){
    return(randf() * (b-a) + a);
}

// Test function to confirm that the RNG has even distribution over the range [0, 1]
void TestRng(int iterations){
    int over = 0;
    int under = 0;
    for(int i = 0; i < iterations; i++){
        float r = randf();
        if(r >= 0.9 && r <= 1.0){
            over++;
        }
        else if(r >= 0 && r <= 0.1){
            under++;
        }
        //std::cout << "Random number: " << randf() << "\n";
    }
    std::cout << "Values between 0.9 and 1.0: " << over << "\n";
    std::cout << "Values between 0.0 and 0.1: " << under << "\n";
}

void NewBranch(int b, int depth){
    gluggScale(BRANCH_SCALE, BRANCH_SCALE, BRANCH_SCALE);
    float rot = float(b)*(2.0/4.0)*3.14 + rand_range(-0.3, 0.3);
    float branch_tilt = rand_range(0.3, 1.0);
    // Rotation around the y-axis (trunk)
    gluggRotate(rot, 0, 1.0, 0);
    // Tilt branch
    gluggRotate(branch_tilt, 1.0, 0, 0);
    if(depth == 0){
        MakeCylinderAlt(20, 2, 0.05, 0.2);
    }
    else{
        MakeCylinderAlt(20, 2, 0.15, 0.2);
    }
    gluggTranslate(0, 1.9, 0);

    int branches = (int)(rand_range(2, 4.99));
    if(depth > 0){
        for(int i = 0; i < branches; i++){
            gluggPushMatrix();
            NewBranch(i, depth-1);
            gluggPopMatrix();
        }
    }
}

gluggModel MakeTree()
{
	gluggSetPositionName("inPosition");
	gluggSetNormalName("inNormal");
	gluggSetTexCoordName("inTexCoord");

	gluggBegin(GLUGG_TRIANGLES);

    // rand test
    // TestRng(500);

    // Make the trunk of the tree

    MakeCylinderAlt(20, 2.5, 0.15, 0.2);
    gluggTranslate(0, 2.4, 0);
    gluggPushMatrix();

    // Draw the rest of the owl

    int branches = (int)(rand_range(2, 4.99));
    for(int i = 0; i < branches; i++){
        gluggPushMatrix();
        NewBranch(i, MAX_DEPTH - (int)rand_range(0.0, 2.99));
        gluggPopMatrix();
    }

	return gluggBuildModel(0);
}



gluggModel tree;

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);

	// Set the clipping volume
	projectionMatrix = perspective(45,1.0f*w/h,1,1000);
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
}

void init(void)
{
	// GL inits
	glClearColor(0.4,0.6,0.8,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300.0);

	// Load and compile shader
	phongShader = loadShaders("phong.vert", "phong.frag");
	texShader = loadShaders("textured.vert", "textured.frag");
	printError("init shader");

	// Upload geometry to the GPU:
	floormodel = LoadDataToModel((vec3 *)vertices2, (vec3 *)normals2, (vec2 *)texcoord2, NULL,
			indices2, 4, 6);

// Important! The shader we upload to must be active!
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);

	glUniform1i(glGetUniformLocation(texShader, "tex"), 0); // Texture unit 0

	LoadTGATextureSimple("grass.tga", &grasstex);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	LoadTGATextureSimple("bark2.tga", &barktex);

	tree = MakeTree();

	printError("init arrays");
}

GLfloat a = 0.0;
vec3 campos = vec3(0, 1.5, 10);
vec3 forward = vec3(0, 0, -4);
vec3 up = vec3(0, 1, 0);

bool jumping = false;
bool falling = false;
float jump_time = 2.0;

void display(void)
{
	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 worldToView, m; // m1, m2, m, tr, scale;

	if (glutKeyIsDown('a'))
		forward = mat3(Ry(0.05))* forward;
	if (glutKeyIsDown('d'))
		forward = mat3(Ry(-0.05)) * forward;
	if (glutKeyIsDown('w'))
		campos = campos + forward * 0.1;
	if (glutKeyIsDown('s'))
		campos = campos - forward * 0.1;
	if (glutKeyIsDown('q'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		campos = campos - side * 0.05;
	}
	if (glutKeyIsDown('e'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		campos = campos + side * 0.05;
	}
	// Jump
	if (glutKeyIsDown(' ') && !jumping)
    {
        jumping = true;
    }
    if (jumping){
        campos = campos + up*0.1;
        jump_time -= 0.1;
        if(jump_time <= 0){
            jumping = false;
            falling = true;
            jump_time = 2.0;
        }
    }
    if (falling){
        campos = campos - up*0.2;
        if(campos.y <= 1.5){
            campos.y = 1.5;
            falling = false;
        }
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
		mat4 m = ArbRotate(side, 0.05);
		forward = mat3(m) * forward;
	}
	if (glutKeyIsDown('-'))
	{
		vec3 side = cross(forward, vec3(0,1,0));
		mat4 m = ArbRotate(side, -0.05);
		forward = m * forward;
	}

	worldToView = lookAtv(campos, campos + forward, up);

	a += 0.1;

	glBindTexture(GL_TEXTURE_2D, grasstex);
	// Floor
	glUseProgram(texShader);
	m = worldToView;
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(floormodel, texShader, "inPosition", "inNormal", "inTexCoord");

	// Draw the tree, as defined on MakeTree
	glBindTexture(GL_TEXTURE_2D, barktex);
	glUseProgram(texShader);
    m = worldToView * T(0, 0, 0);
    glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	gluggDrawModel(tree, texShader);

	printError("display");

	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
	switch (key)
	{
		case ' ':
			forward.y = 0;
			forward = normalize(forward) * 4.0;
			break;
	}
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(640,360);
	glutCreateWindow("Fractal tree lab");
	glutRepeatingTimer(20);
	glutDisplayFunc(display);
	glutKeyboardFunc(keys);
	glutReshapeFunc(reshape);
	init ();
	glutMainLoop();
}
