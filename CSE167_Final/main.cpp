#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>
#include <assert.h>

#include "core.h"

#include <glut/glut.h>
#include "Vector3.h"
#include "Matrix4.h"
#include "OParticles.h"
#include "Collision.h"
#include "Light.h"
#include "Quat.h"

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
	phi = 10;
	psi = 35;
	view_pos = Vector3(-0.79,2.58,0.67);
	speed    = Vector3(0, 0, 0);
	time = 0;
	fps = 60;

	mouseX = 0;
	mouseY = 0;
	mouseButton = 0;
}

static void initCore()
{
	Core::AddDirectoryPath(
		"data/,"
		"data/engine/,"
		"data/textures/,"
		"data/textures/cube/,"
		"data/materials/,"
		"data/shaders/,"
		"data/meshes/,"
		"data/testing/");

	p_collision = new Collision();
	int isInit = Core::init(win_x, win_y);
	assert(isInit && "Core::init failed()");

	Core::LoadScene("testing.map");
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
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(projection.getPointer());
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(modelview.getPointer());

	Core::Update(spf);
	Core::RenderScene(spf);
	glutSwapBuffers();
}

static void reshape(int x, int y)
{
	win_x = x; 
	win_y = y;
	glViewport(0, 0, win_x, win_y);
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
		Object *intersectedObj = 
			Core::GetIntersectObject(view_pos, view_pos.add(direction.scale(1000)), &pt, &n);

		if(intersectedObj != nullptr)
		{
			//TODO: add physics support later
			//p_bullet_explosion->
		}
	}

	//camera
	Matrix4 m0,m1,m2;
	//	

	speed = speed - speed.scale(5).scale(spf);

	Quat q0,q1;
	q0.set(Vector3(0,0,1),-psi);
	q1.set(Vector3(0,1,0),phi);
	direction = (q0 * q1).to_matrix().multiplyVec3(Vector3(1,0,0));
	Vector3 x,y,z;
	x = direction;
	y = Vector3::Cross(direction, Vector3(0,0,1));
	y.normalize();
	z = Vector3::Cross(y,x);
	view_pos = view_pos + (x.scale(speed['x']) + y.scale(speed['y']) + z.scale(speed['z'])).scale(spf);

	/*for(int i = 0; i < 4; i++) {
	collide->collide(NULL,camera + (x * speed.x + y * speed.y + z * speed.z) * ifps / 4.0f,0.25);
	for(int j = 0; j < collide->num_contacts; j++) {
	camera += collide->contacts[j].normal * collide->contacts[j].depth / (float)collide->num_contacts;
	}
	}*/

	modelview.setLookAtMat(view_pos, view_pos + direction, Vector3(0,0,1));
	projection.setPerspectiveMat(89, (float)win_x / (float)win_y, 0.1, 500);

	glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
	int vel = 20;
	switch(key)
	{
	case 'w':
		speed.set(speed['x'] + vel * spf, 'x');
		break;

	case 's':
		speed.set(speed['x'] - vel * spf, 'x');
		break;

	case 'a':
		speed.set(speed['y'] - vel * spf, 'x');
		break;

	case 'd':
		speed.set(speed['y'] + vel * spf, 'x');
		break;
	case 'u':
		phi++;
		psi--;
		break;
	}

}

static void specialKey(int key, int x, int y)
{

}

static void mouse(int btn, int state, int x, int y)
{
	mouseButton = btn;
	mouseX = x;
	mouseY = y;

	/*if(btn == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_DOWN)
		{
			mouseX = win_x / 2;
			mouseY = win_y / 2;
		}
	}*/

}

static void mouseMotion(int x, int y)
{
	mouseX = x;
	mouseY = y;
	/*psi += (mouseX - win_x / 2) * 0.2;
	phi += (mouseY - win_y / 2) * 0.2;
	if(phi < -89) phi = -89;
	if(phi > 89) phi = 89;
*/
}

static void setup()
{
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}


int main(int argc, char *argv[])
{
	initVars();
	glutInit(&argc, argv);
	glutInitWindowSize(win_x, win_y);
	glutCreateWindow("CSE167_Final Project");

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	initCore();

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