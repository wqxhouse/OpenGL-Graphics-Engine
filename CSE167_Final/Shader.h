#pragma once

#include <map>
#include "GLee.h"
#include "Parser.h"

/*
*/
class Vector4;
class Texture;
class Shader
{
public:

	Shader();
	Shader(const char *name);
	virtual ~Shader();

	// load shaders
	virtual int loadVertex(const char *src);
	virtual int loadFragment(const char *src);

	// compile shader
	virtual int compile();

	// clear shader
	virtual void clear();

	// find parameter
	virtual int findParameter(const char *name);

	// set parameter by id
	virtual void setParameterBool(int location, int value);
	virtual void setParameterInt(int location, const int *value,int size);
	virtual void setParameterFloat(int location, const float *value,int size);
	virtual void setParameterFloatArray(int location, const float *value,int size,int num);

	// set parameter by name
	virtual void setParameterBool(const char *name,int value);
	virtual void setParameterInt(const char *name,const int *value,int size);
	virtual void setParameterFloat(const char *name,const float *value,int size);
	virtual void setParameterFloatArray(const char *name,const float *value,int size,int num);

	// set shader parameters
	void bind() ;


	// enable / disable shaders
	void enable() ;
	void disable() ;

	// get shader
	GLuint getProgramID() const;

	void bindTexture(int unit, Texture *texture);
	static Shader *old_shader;

	enum 
	{
		NUM_MATRIXES = 4,
		NUM_PARAMETERS = 2,
		NUM_LOCAL_PARAMETERS = 4,
		NUM_TEXTURES = 6,
	};


private:

	std::map<std::string, int> parameter_names_;	// parameter id by name

	GLuint program_id_;							    // GLSL program id



	Parser parser_;

	void parseShaderFile(const char *src);
	void setShaderParameters();

	static Vector4 parameters[2];
	static Texture *old_textures[6];
};

