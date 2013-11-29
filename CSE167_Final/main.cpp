#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <assert.h>
#include <glut/glut.h>

#include "Vector3.h"
#include "Matrix4.h"
#include "OParticles.h"
#include "Collision.h"
#include "Light.h"
#include "Core.h"

int win_x;
int win_y;
int mouseX;
int mouseY;
int mouseButton;

float fps;
float spf;

float time;
float start_t;
float end_t;
int counter;
int fpsLock;
float phi,psi;					
Vector3 view_pos;
Vector3 speed;
Vector3 direction;

Collision *p_collision;

Matrix4 modelview;					
Matrix4 projection;

Light *p_gun_light;				
OParticles *p_bullet_explosion;

static void initVars();
static int getTime();
static void display();
static void reshape(int x, int y);
static void idle();
static void keyboard(unsigned char key, int x, int y);
static void specialKey(int key, int x, int y);
static void mouse(int btn, int state, int x, int y);
static void mouseMotion(int x, int y);
static void setup();

static void initVars()
{
	win_x = 1280;
	win_y = 720;
	start_t = 0;
	end_t = 0;
	counter = 0;
	fpsLock = 60;
	phi = 35;
	psi = 10;
	view_pos = Vector3(-0.79,2.58,0.67);
	speed    = Vector3(0, 0, 0);
	time = 0;

	mouseX = 0;
	mouseY = 0;
	mouseButton = 0;

	p_collision = new Collision();
	int isInit = Core::init(win_x, win_y);
	assert(isInit && "Core::init failed()");

	Core::AddDirectoryPath( "res/,"
							"res/textures/,"
							"res/textures/cube/,"
							"res/materials/,"
							"res/shaders/,"
							"res/meshes/");
	Core::LoadScene("world.sce");
	printf("Scene Load Completed...\n");
	
}

static int getTime() 
{
	static int base;
	static int initialized = 0;
	if(!initialized) 
	{
		base = timeGetTime();
		initialized = 1;
	}
	return timeGetTime() - base;
}


static void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glutSwapBuffers();
}

static void reshape(int x, int y)
{

}

static void idle()
{
	if(counter++ == 10) 
	{
		end_t = start_t;
		start_t = getTime();
		fps = counter * 1000.0f / (float)(start_t - end_t);
		counter = 0;
	}
	spf = 1.0f / fps;
	time += spf;
	//////////////////////////////////////////////////////////////////////////
	// Handle gun fire ///////////////////////////////////////////////////////
	static bool fire = false;
	if(!fire && mouseButton & GLUT_LEFT_BUTTON)
	{
		Vector3 pt; 
		Vector3 n;
		Object *intersectedObj = Core::GetIntersectObject(view_pos, view_pos.add(direction.scale(1000)), &pt, &n);
		if(intersectedObj != nullptr)
		{
			//TODO: add physics support later
			//p_bullet_explosion->
		}
	}
}

static void keyboard(unsigned char key, int x, int y)
{

}

static void specialKey(int key, int x, int y)
{

}

static void mouse(int btn, int state, int x, int y)
{
	mouseButton = btn;
	mouseX = x;
	mouseY = y;
}

static void mouseMotion(int x, int y)
{
	mouseX = x;
	mouseY = y;
}

static void setup()
{
	glClearColor(0, 0, 0, 1);
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);

	glutInitWindowSize(win_x, win_y);
	glutCreateWindow("CSE167_Final Project");

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKey);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);

	setup();
	glutMainLoop();
}