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

bool isMouseHidden;
std::vector<bool> keyStates;
bool isFullScreen;

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
	win_x = 1024;
	win_y = 768;
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

	keyStates.resize(256);
	isMouseHidden = true;

	isFullScreen = false;
}

static void initCore()
{
	//hide cursor
	glutSetCursor(GLUT_CURSOR_NONE); 

	Core::AddDirectoryPath(
		"data/scene/");

	p_collision = new Collision();
	int isInit = Core::init(win_x, win_y);
	assert(isInit && "Core::init failed()");

	Core::LoadScene("scene.map");
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

	//handle keypresses/////////
	float vel = 100;
	if(keyStates[(int)'w']) speed.set(speed['x'] + vel * spf, 'x');
	if(keyStates[(int)'s']) speed.set(speed['x'] - vel * spf, 'x');
	if(keyStates[(int)'a']) speed.set(speed['y'] - vel * spf, 'y');
	if(keyStates[(int)'d']) speed.set(speed['y'] + vel * spf, 'y');
	if(keyStates[(int)'k']) psi++;
	if(keyStates[(int)'h']) psi--;
	if(keyStates[(int)'u'])
	{
		phi+=0.5;
		if(phi <= -89.0f)
		{
			phi = -89;
		}
	}
	if(keyStates[(int)'j'])
	{
		phi+=0.5;
		if(phi >= 89.0f)
		{
			phi = 89;
		}
	}
	if(keyStates[(int)'l'])
	{
		Core::useShadowBit = Core::useShadowBit ? false : true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Handle gun fire ///////////////////////////////////////////////////////
	//static bool fire = false;
	//if(!fire && mouseButton & GLUT_LEFT_BUTTON)
	//{
	//	Vector3 pt; 
	//	Vector3 n;
	//	Object *intersectedObj = 
	//		Core::GetIntersectObject(view_pos, view_pos.add(direction.scale(1000)), &pt, &n);

	//	if(intersectedObj != nullptr)
	//	{
	//		//TODO: add physics support later
	//		//p_bullet_explosion->
	//	}
	//}

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

	static int cc = 0;
	if(cc ++ == 10000)
	{
		view_pos.print();
		cc = 0;
	}
	for(int i = 0; i < 4; i++) {
		p_collision->collide(nullptr, view_pos + (x.scale(speed['x']) + y.scale(speed['y']) + z.scale(speed['z'])).scale(spf).scale( 1.0f /4.0f), 0.25);
		for(int j = 0; j < p_collision->num_contacts; j++) 
		{
			view_pos = view_pos + p_collision->contacts[j].normal.scale( p_collision->contacts[j].depth ).scale( (float)p_collision->num_contacts );
		}
	}

	modelview.setLookAtMat(view_pos, view_pos + direction, Vector3(0,0,1));
	projection.setPerspectiveMat(89, (float)win_x / (float)win_y, 0.1, 500);

	glutPostRedisplay();
}

static void keyUp(unsigned char key, int x, int y)
{
	keyStates[key] = false;
}

static void keyboard(unsigned char key, int x, int y)
{
	keyStates[key] = true;
}

static void specialKey(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_F11:
		isFullScreen = !isFullScreen;
		if (isFullScreen) 
		{
			glutFullScreen();
		}
		else {
			glutReshapeWindow(1024, 768);
			glutPositionWindow(100, 100);
		}
		break;
	}
}

static void mouse(int btn, int state, int x, int y)
{
	if(btn == GLUT_LEFT_BUTTON)
	{
		if(state == GLUT_UP)
		{
			isMouseHidden = true;
			glutSetCursor(GLUT_CURSOR_NONE); 
		}
	}


	if(btn == GLUT_RIGHT_BUTTON)
	{
		if(state == GLUT_UP)
		{
			isMouseHidden = false;
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW); 
		}
	}
}

static void passiveMouseMotion(int x, int y)
{
	if(isMouseHidden)
	{
		int centerX = win_x / 2;
		int centerY = win_y / 2;

		int deltaX =  x - centerX;
		int deltaY =  y - centerY;

		if(deltaX != 0 || deltaY != 0) 
		{
			psi += deltaX * 0.2f;
			phi += deltaY  * 0.2f;
			if(phi < -89) phi = -89;
			if(phi > 89) phi = 89;
			glutWarpPointer(centerX, centerY);
		}
	}
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

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

	initCore();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp);
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);
	glutSpecialFunc(specialKey);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);

	glutMouseFunc(mouse);
	glutPassiveMotionFunc(passiveMouseMotion);

	setup();
	glutMainLoop();
}