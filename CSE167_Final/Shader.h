#pragma once

#include "core.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"

class Shader 
{
public:
	
	Shader(const char *name);
	~Shader();
	
	void load(const char *name);
	
	static void setParameter(int num,const Vector4 &parameter);
	
	void enable();
	void disable();
	void bind();
	
	void bindTexture(int unit,Texture *texture);
	
	enum 
	{
		NUM_MATRIXES = 4,
		NUM_PARAMETERS = 2,
		NUM_LOCAL_PARAMETERS = 4,
		NUM_TEXTURES = 6,
	};
	
	static Shader *old_shader;
	
protected:
	
	enum 
	{
		TIME = 1,
		SIN,
		COS,
		CAMERA,
		ICAMERA,
		LIGHT,
		ILIGHT,
		LIGHT_COLOR,
		FOG_COLOR,
		VIEWPORT,
		PARAMETER,
		PROJECTION,
		MODELVIEW,
		IMODELVIEW,
		TRANSFORM,
		ITRANSFORM,
		LIGHT_TRANSFORM,
	};
	
	struct Matrix 
	{
		int num;	// matrix number
		int type;	// matrix type
	};
	
	struct LocalParameter 
	{
		int num;		// parameter number
		int type;		// parameter type
		int parameter;
	};
	
	GLuint compileARBtec(const char *src);
	
	void getMatrix(const char *name,Matrix *m);
	void getLocalParameter(const char *name,LocalParameter *p);
	
	int num_matrixes;
	Matrix matrixes[NUM_MATRIXES];
	
	int num_vertex_parameters;
	LocalParameter vertex_parameters[NUM_LOCAL_PARAMETERS];
	
	int num_fragment_parameters;
	LocalParameter fragment_parameters[NUM_LOCAL_PARAMETERS];
	
	GLuint vertex_target_;
	GLuint vertex_id_;
	GLuint fragment_target_;
	GLuint fragment_id_;
	
	static Vector4 parameters[NUM_PARAMETERS];
	static Texture *old_textures[Shader::NUM_TEXTURES];
};

