
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

long long seed = 12744274;

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

void MakeBranch(int depth, int variation){
    // Rescale
    gluggScale(0.75, 0.75, 0.75);
    // Tilt the branch
    gluggRotate(0.4, 1.0, 0.0, 0.0);
    // Rotate around x-axis
    gluggRotate(1.5*variation, 0.0, 1.0, 0.0);
    MakeCylinderAlt(20, 2, 0.2, 0.2);

    gluggTranslate(0, 1.8, 0.0);

    int new_depth = depth - 1;
    if(new_depth >= 0){
        for(int i = 0; i < 4; i++){
            gluggPushMatrix();
            MakeBranch(new_depth, i);
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

    // Try to make a tree without loops/recursion to figure out the logic

    // rand test
    TestRng(500);

    MakeCylinderAlt(20, 2, 0.2, 0.2);
    gluggPushMatrix();
    gluggTranslate(0, 2, 0);
    MakeCylinderAlt(20, 3, 0.1, 0.2);
    gluggPopMatrix();

    gluggTranslate(0, 2, 0);
    gluggPushMatrix();
    // rotation around trunk
    gluggRotate(rand_ang(), 0, 1.0, 0);
    gluggRotate(1.0, 1.0, 0, 0);
    MakeCylinderAlt(10, 2.5, 0.01, 0.1);
    gluggPopMatrix();

    gluggTranslate(0, 0.5, 0);
    gluggPushMatrix();
    // rotation around trunk
    gluggRotate(rand_ang(), 0, 1.0, 0);
    gluggRotate(1.0, 1.0, 0, 0);
    MakeCylinderAlt(10, 2.5, 0.01, 0.1);
    gluggPopMatrix();

    //gluggTranslate()

	// Between gluggBegin and gluggBuildModel, call MakeCylinderAlt plus glugg transformations
	// to create a tree.

	//int depth = 6;

	// Create the trunk
	//MakeCylinderAlt(20, 2, 0.2, 0.2);
	// Move to the top of the trunk
	//gluggTranslate(0, 1.8, 0.0);

	// Step 1: Make three diverging branches

	//MakeBranch(depth, 0);

	/*
	for(int i = 0; i <= depth; i++){
        gluggPushMatrix();
        for(int j = 0; j <= 5; j++){
            // Tilt the branch
            gluggRotate(0.4, 1.0, 0.0, 0.0);
            float rot = j*1.5;
            gluggRotate(rot, 0.0, 1.0, 0.0);
            MakeCylinderAlt(20, 2, 0.15, 0.2);
        }
        gluggPopMatrix();
        //gluggTranslate(0, 1.8, 0.0);
	}
	*/


	//gluggTranslate(0, 2.0, 0.0);
	//gluggRotate(0.5, 1.0, 0, 0);
	//MakeCylinderAlt(20, 5, 0.05, 0.15);

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
        printf("Jumping");
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
