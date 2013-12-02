#pragma once
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "BasicMath.h"

class Position;
class WorldFile
{
public:
	static void load(const char *name);

protected:

	static char *data;	

	static const char *error(const char *error,...);

	static const char *read_token(char *must = nullptr);
	static int read_bool();
	static int read_int();
	static float read_float();
	static Vector3 read_vec3();
	static Vector4 read_vec4();
	static const char *read_string();

	static void load_bsp();
	static void load_pos(Position &pos, Matrix4 &matrix);
	static void load_light();
	static void load_fog();
	static void load_mesh();
	static void load_particles();
};

