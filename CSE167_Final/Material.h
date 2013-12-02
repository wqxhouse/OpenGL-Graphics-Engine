#pragma once
#include "core.h"
#include "Shader.h"

class Material
{
public:
	enum 
	{
		NUM_MATRIXES = 4,
		NUM_PARAMETERS = 2,
		NUM_LOCAL_PARAMETERS = 4,
		NUM_TEXTURES = 6,
	};

	Material(const char *name);
	~Material();

	void load(const char *name);

	int enable();
	void disable();
	void bind();

	void bindTexture(int unit,Texture *texture);

	int blend_;

	GLuint sfactor_;
	GLuint dfactor_;

	int alpha_test_;

	GLuint alpha_func_;
	float alpha_ref_;

	GLuint getBlendFactor(const char *factor);
	GLuint getAlphaFunc(const char *func);

	Shader *light_shader_;
	Shader *ambient_shader_;
	Vector4 parameters_[Shader::NUM_PARAMETERS];
	Texture *textures_[Shader::NUM_TEXTURES];

	static Material *old_material_;
};

