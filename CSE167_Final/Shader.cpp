#include "Shader.h"
#include <assert.h>
#include <string>
#include "ShaderConstants.h"
#include "core.h"
#include "Texture.h"
#include "Vector4.h"
#include "Light.h"


Shader *Shader::old_shader;
Vector4 Shader::parameters[NUM_PARAMETERS];
Texture *Shader::old_textures[NUM_TEXTURES];

Shader::Shader(const char *name)
	: parser_(name)
{
	program_id_ = 0;
	clear();

	char *vertex_src = parser_.get("vertex");
	char *fragment_src = parser_.get("fragment");
	
	loadVertex(vertex_src);
	loadFragment(fragment_src);
	compile();
}

Shader::~Shader()
{
	clear();
}

/*
 */
int Shader::loadVertex(const char *src) 
{
	// create shader
	GLhandleARB vertex_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER);
	
	// compile shader
	GLint status = 0;
	glShaderSourceARB(vertex_shader,1,(const GLchar**)&src, nullptr);
	glCompileShaderARB(vertex_shader);
	glGetShaderiv(vertex_shader,GL_COMPILE_STATUS,&status);
	if(status == GL_FALSE) 
	{
		char error[32768];
		glGetShaderInfoLog(vertex_shader,sizeof(error), nullptr,error);
		fprintf(stderr, "Vertex shader error log: %s\n", error);

		//glDeleteObjectARB(vertex_shader);
		//delete [] src;
		return 0;
	}
	
	// attach shader
	if(program_id_ == 0)
	{
		program_id_ = glCreateProgramObjectARB();
	}
	glAttachObjectARB(program_id_,vertex_shader);
	//glDeleteObjectARB(vertex_shader);
	
	// set vertex attributes
	glBindAttribLocation(program_id_,0,"att_0");
	glBindAttribLocation(program_id_,1,"att_1");
	glBindAttribLocation(program_id_,2,"att_2");
	glBindAttribLocation(program_id_,3,"att_3");
	glBindAttribLocation(program_id_,4,"att_4");
	glBindAttribLocation(program_id_,5,"att_5");
	glBindAttribLocation(program_id_,6,"att_6");
	glBindAttribLocation(program_id_,7,"att_7");
	glBindAttribLocation(program_id_,8,"att_8");
	glBindAttribLocation(program_id_,9,"att_9");
	glBindAttribLocation(program_id_,10,"att_10");
	glBindAttribLocation(program_id_,11,"att_11");
	glBindAttribLocation(program_id_,12,"att_12");
	glBindAttribLocation(program_id_,13,"att_13");
	glBindAttribLocation(program_id_,14,"att_14");
	glBindAttribLocation(program_id_,15,"att_15");
	
	//delete [] src;
	return 1;
}

int Shader::loadFragment(const char *src)
{
	// create shader
	GLhandleARB fragment_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
	
	// compile shader
	GLint status = 0;
	glShaderSourceARB(fragment_shader,1,(const GLchar**)&src, nullptr);
	glCompileShaderARB(fragment_shader);
	glGetShaderiv(fragment_shader,GL_COMPILE_STATUS,&status);
	if(status == GL_FALSE) 
	{
		char error[32768];
		glGetShaderInfoLog(fragment_shader,sizeof(error), nullptr, error);
		fprintf(stderr, "Fragment shader error log: %s\n", error);
		//glDeleteObjectARB(fragment_shader);
		//delete [] src;
		return 0;
	}
	
	// attach shader
	if(program_id_ == 0)
	{
		program_id_ = glCreateProgramObjectARB();
	}
	glAttachObjectARB(program_id_,fragment_shader);
	//glDeleteObjectARB(fragment_shader);
	
	//delete [] src;
	return 1;
}


int Shader::compile() 
{
	if(program_id_)
	{
		// link program
		GLint status = 0;
		glLinkProgramARB(program_id_);
		glGetProgramiv(program_id_,GL_LINK_STATUS,&status);
		if(status == GL_FALSE) 
		{
			char error[32768];
			glGetProgramInfoLog(program_id_,sizeof(error),nullptr,error);
			printf("Shader::compile(): %s",error);
			//glDeleteObjectARB(1, &program_id_);
			program_id_ = 0;
			return 0;
		}
		
		// set textures
		GLint old_program_id = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &old_program_id);
		glUseProgramObjectARB(program_id_);
		for(int i = 0; i < 32; i++) 
		{
			char vertex[128];
			sprintf(vertex,"s_vertex_%d",i);
			GLint location = glGetUniformLocationARB(program_id_,vertex);
			if(location >= 0) glUniform1i(location,i);
		}
		for(int i = 0; i < 32; i++) 
		{
			char texture[128];
			sprintf(texture,"s_texture_%d",i);
			GLint location = glGetUniformLocationARB(program_id_,texture);
			if(location >= 0) glUniform1i(location,i);
		}
		glUseProgramObjectARB(old_program_id);
		
		// validate program
		glValidateProgramARB(program_id_);
		glGetProgramiv(program_id_,GL_VALIDATE_STATUS,&status);
		if(status == GL_FALSE) 
		{
			char error[32768];
			glGetProgramInfoLog(program_id_,sizeof(error),NULL,error);
			printf("Shader::compile(): %s",error);
			//glDeleteObjectARB(program_id_);
			program_id_ = 0;
			return 0;
		}
	}
	
	return 1;
}

void Shader::clear() 
{
	parameter_names_.clear();

	if(glIsProgramARB(program_id_))
	{
		//glDeleteObjectARB(program_id_);
	}
	
	program_id_ = 0;
}

int Shader::findParameter(const char *name) 
{
	std::map<std::string, int>::iterator it = parameter_names_.find(name);
	if(it != parameter_names_.end())
	{
		return it->second;
	}
	if(program_id_) 
	{
		GLint location = glGetUniformLocationARB(program_id_, name);
		if(location != -1) 
		{
			parameter_names_.emplace(std::make_pair(name, location));
			return location;
		}
	}
	parameter_names_.emplace(std::make_pair(name, -1));
	return -1;
}

void Shader::setParameterBool(int location,int value) 
{
	glUniform1i(location,value);
}

void Shader::setParameterInt(int location,const int *value,int size) 
{
	switch(size) 
	{
		case 1: glUniform1iv(location,1,value); break;
		case 2: glUniform2iv(location,1,value); break;
		case 3: glUniform3iv(location,1,value); break;
		case 4: glUniform4iv(location,1,value); break;
		default: assert(0 && "Shader::setParameterInt(): bad parameter size");
	}
}

void Shader::setParameterFloat(int location,const float *value,int size) 
{
	switch(size) 
	{
		case 1: glUniform1fv(location,1,value); break;
		case 2: glUniform2fv(location,1,value); break;
		case 3: glUniform3fv(location,1,value); break;
		case 4: glUniform4fv(location,1,value); break;
		case 9: glUniformMatrix3fv(location,1,GL_FALSE,value); break;
		case 16: glUniformMatrix4fv(location,1,GL_FALSE,value); break;
		default: assert(0 && "Shader::setParameterFloat(): bad parameter size");
	}
}

void Shader::setParameterFloatArray(int location,const float *value,int size,int num) 
{
	switch(size)
	{
		case 1: glUniform1fv(location,num,value); break;
		case 2: glUniform2fv(location,num,value); break;
		case 3: glUniform3fv(location,num,value); break;
		case 4: glUniform4fv(location,num,value); break;
		case 9: glUniformMatrix3fv(location,num,GL_FALSE,value); break;
		case 16: glUniformMatrix4fv(location,num,GL_FALSE,value); break;
		default: assert(0 && "Shader::setParameterFloatArray(): bad parameter size");
	}
}
void Shader::setParameterBool(const char *name,int value) 
{
	int id = findParameter(name);
	//if(id == -1) printf("Shader::setParameterBool(): can't find \"%s\" parameter\n",name);
	//else 
		setParameterBool(id,value);
}

void Shader::setParameterInt(const char *name,const int *value,int size) {
	int id = findParameter(name);
	//if(id == -1) printf("Shader::setParameterInt(): can't find \"%s\" parameter\n",name);
	//else 
		setParameterInt(id,value,size);
}

void Shader::setParameterFloat(const char *name,const float *value,int size) {
	int id = findParameter(name);
	//if(id == -1) printf("Shader::setParameterFloat(): can't find \"%s\" parameter\n",name);
	//else 
		setParameterFloat(id,value,size);
}

void Shader::setParameterFloatArray(const char *name,const float *value,int size,int num) {
	int id = findParameter(name);
//	if(id == -1) printf("Shader::setParameterFloatArray(): can't find \"%s\" parameter\n",name);
	//else 
		setParameterFloatArray(id,value,size,num);
}

/*
 */
void Shader::enable()  
{
	if(old_shader == this) return;
	if(old_shader) 
	{
		old_shader->disable();
		if(program_id_)
		{
			glUseProgramObjectARB(program_id_);	
		}
	}
	else 
	{
		if(program_id_)
		{
			glUseProgramObjectARB(program_id_);	
		}
	}
}

void Shader::disable()  
{
	if(program_id_) glUseProgramObjectARB(0);

	for(int i = 0; i < 6; i++) 
	{
		if(old_textures[i]) 
		{
			glActiveTexture(GL_TEXTURE0 + i);
			old_textures[i]->disable();
		}
		old_textures[i] = NULL;
	}
	glActiveTexture(GL_TEXTURE0);
	old_shader = nullptr;
}

void Shader::bind()  
{
	if(old_shader != this) 
	{
		old_shader = this;
	}

	setShaderParameters();
}

/*
 */
GLuint Shader::getProgramID() const 
{
	return program_id_;
}

void Shader::setShaderParameters()
{
	setParameterFloat(SHADER_PROJECTION, Core::projection_.getPointer(), 16);
	setParameterFloat(SHADER_MODELVIEW, Core::modelview_.getPointer(), 16);
	setParameterFloat(SHADER_IMODELVIEW, Core::transform_.getPointer(), 16);
	setParameterFloat(SHADER_TRANSFORM, Core::transform_.getPointer(), 16);
	setParameterFloat(SHADER_ITRANSFORM, Core::itransform_.getPointer(), 16);

	setParameterFloat(SHADER_CAMERA_POSITION,Core::light_pos_.getPointer(), 4);
	setParameterFloat(SHADER_CAMERA_INVERSE, Vector4(Core::itransform_.multiplyVec3(Core::camera_.getPosCoord()), 1).getPointer(), 16);
	//setParameterFloat(SHADER_CAMERA_DIRECTION, Core::direction_, 3);

	//lights
	setParameterFloat(SHADER_LIGHT_POSITION, Core::light_pos_.getPointer(),4);
	setParameterFloat(SHADER_LIGHT_POS_INVERSE, Vector4(Core::itransform_.multiplyVec3(Core::light_pos_.getVector3()), Core::light_pos_['w']).getPointer(), 4);
	//setParameterFloat(SHADER_LIGHT_DIRECTION,light_direction,3);
	setParameterFloat(SHADER_LIGHT_COLOR, Core::light_color_.getPointer(),4);

	if(Core::curr_light_ != nullptr)
	{
		setParameterFloat(SHADER_LIGHT_IRADIUS, Vector4(1.0f / Core::curr_light_->radius(),
			1.0f / Core::curr_light_->radius(),
			1.0f / Core::curr_light_->radius(),
			1).getPointer(), 4);
	}
	
	// light transformation
	setParameterFloat(SHADER_LIGHT_TRANSFORM, Core::curr_light_->transform_.getPointer(), 16);

	//shadow parameters
	/*setParameterFloat(SHADER_LIGHT_SHADOW_OFFSET,light_shadow_offset,3);
	setParameterFloatArray(SHADER_LIGHT_SHADOW_OFFSETS,light_shadow_offsets[0],4,4);
	setParameterFloat4(SHADER_LIGHT_SHADOW_IRADIUS,Math::rcpf(light_shadow_radius),light_shadow_ambient,light_shadow_softness * 2.0f,light_shadow_softness * 4.0f);
	setParameterFloat(SHADER_LIGHT_SHADOW_DEPTH_BIAS,light_shadow_depth_bias / 1000.0f,3);
	setParameterFloat(SHADER_LIGHT_SHADOW_DEPTH_RANGE,light_shadow_depth_range,2);
	setParameterFloat(SHADER_LIGHT_SHADOW_PROJECTION,light_shadow_projections[0],16);
	setParameterFloatArray(SHADER_LIGHT_SHADOW_PROJECTIONS,light_shadow_projections[0],16,4);*/
}

void Shader::bindTexture(int unit, Texture *texture)
{
	if(old_textures[unit] == texture) return;
	glActiveTexture(GL_TEXTURE0 + unit);
	if(texture)
	{
		if(old_textures[unit] == NULL) texture->enable();
		else if(texture->target != old_textures[unit]->target) 
		{
			old_textures[unit]->disable();
			texture->enable();
		}
		texture->bind();
	} else 
	{
		if(old_textures[unit]) old_textures[unit]->disable();
	}
	old_textures[unit] = texture;
}