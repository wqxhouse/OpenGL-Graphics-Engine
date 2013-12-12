#include "Material.h"
#include "Texture.h"
#include "Parser.h"
#include "core.h"
#include "Light.h"
/*
 */
Material *Material::old_material_;

/*
 */
Material::Material(const char *name) 
{
	load(name);
}

Material::~Material()
{

}

/*****************************************************************************/
/*                                                                           */
/* load material                                                             */
/*                                                                           */
/*****************************************************************************/

/*
 */
void Material::load(const char *name) 
{
	blend_ = 0;
	alpha_test_ = 0;
	light_shader_ = nullptr;
	ambient_shader_ = nullptr;
	Parser *parser = new Parser(name);
	if(parser->get("blend")) 
	{
		blend_ = 1;
		char s[1024];
		char d[1024];
		sscanf(parser->get("blend"),"%s %s",s,d);
		sfactor_ = getBlendFactor(s);
		dfactor_ = getBlendFactor(d);
	}
	if(parser->get("alpha_test"))
	{
		alpha_test_ = 1;
		char func[1024];
		sscanf(parser->get("alpha_test"),"%s %f",func,&alpha_ref_);
		alpha_func_ = getAlphaFunc(func);
	}
	/*for(int i = 0; i < Shader::NUM_PARAMETERS; i++) 
	{
	parameters_[i] = Vector4(0,0,0,0);
	char buf[1024];
	sprintf(buf,"parameter%d",i);

	float xx, yy, zz, ww;
	if(parser->get(buf)) sscanf(parser->get(buf),"%f %f %f %f", &xx,&yy,&zz,&ww);
	parameters_[i].set(xx, 'x');
	parameters_[i].set(yy, 'y');
	parameters_[i].set(zz, 'z');
	parameters_[i].set(ww, 'w');
	}*/
	if(parser->get("light_shader")) 
	{
		light_shader_ = Core::LoadShader(parser->get("light_shader"));
	}
	if(parser->get("ambient_shader"))
	{
		ambient_shader_ = Core::LoadShader(parser->get("ambient_shader"));
	}
	if(parser->get("shader")) 
	{
		light_shader_ = ambient_shader_ = Core::LoadShader(parser->get("shader"));
	}
	for(int i = 0; i < Shader::NUM_TEXTURES; i++) 
	{
		textures_[i] = nullptr;
		char buf[1024];
		sprintf(buf,"texture%d",i);
		char *s = parser->get(buf);
		if(s) 
		{
			GLuint target = Texture::TEXTURE_2D;
			int flag = 0;
			int format = 0;
			int filter = 0;
			char name[1024];
			char *d = name;	// read name
			while(*s != '\0' && !strchr(" \t\n\r",*s)) *d++ = *s++;
			*d = '\0';
			d = buf;	// read flags
			while(*s != '\0' && *s != '\n' && strchr(" \t\r",*s)) s++;
			while(1) 
			{
				if(*s == '\0' || strchr(" \t\n\r",*s)) 
				{
					if(d == buf) break;
					*d = '\0';
					d = buf;
					while(*s != '\n' && *s != '\0' && strchr(" \t\r",*s)) s++;
					
					if(!strcmp(buf,"2D")) target = Texture::TEXTURE_2D;
					else if(!strcmp(buf,"RECT")) target = Texture::TEXTURE_RECT;
					else if(!strcmp(buf,"CUBE")) target = Texture::TEXTURE_CUBE;
					else if(!strcmp(buf,"3D")) target = Texture::TEXTURE_3D;
					
					else if(!strcmp(buf,"LUMINANCE")) format = Texture::LUMINANCE;
					else if(!strcmp(buf,"LUMINANCE_ALPHA")) format = Texture::LUMINANCE_ALPHA;
					else if(!strcmp(buf,"RGB")) format = Texture::RGB;
					else if(!strcmp(buf,"RGBA")) format = Texture::RGBA;
					
					else if(!strcmp(buf,"CLAMP")) flag |= Texture::CLAMP;
					else if(!strcmp(buf,"CLAMP_TO_EDGE")) flag |= Texture::CLAMP_TO_EDGE;
					else if(!strcmp(buf, "REPEAT")) flag |= Texture::REPEAT;
					
					else if(!strcmp(buf,"NEAREST")) filter = Texture::NEAREST;
					else if(!strcmp(buf,"LINEAR")) filter = Texture::LINEAR;
					else if(!strcmp(buf,"NEAREST_MIPMAP_NEAREST")) filter = Texture::NEAREST_MIPMAP_NEAREST;
					else if(!strcmp(buf,"LINEAR_MIPMAP_NEAREST")) filter = Texture::LINEAR_MIPMAP_NEAREST;
					else if(!strcmp(buf,"LINEAR_MIPMAP_LINEAR")) filter = Texture::LINEAR_MIPMAP_LINEAR;
					
					else if(!strcmp(buf,"ANISOTROPY_1")) flag |= Texture::ANISOTROPY_1;
					else if(!strcmp(buf,"ANISOTROPY_2")) flag |= Texture::ANISOTROPY_2;
					else if(!strcmp(buf,"ANISOTROPY_4")) flag |= Texture::ANISOTROPY_4;
					else if(!strcmp(buf,"ANISOTROPY_8")) flag |= Texture::ANISOTROPY_8;
					else if(!strcmp(buf,"ANISOTROPY_16")) flag |= Texture::ANISOTROPY_16;
					
					else fprintf(stderr,"Material::Material(): unknown texture%d flag \"%s\"\n",i,buf);
				} 
				else *d++ = *s++;
			}
			textures_[i] = Core::LoadTexture(name,target,flag | (format == 0 ? Texture::RGB : format) | (filter == 0 ? Core::texture_filter_ : filter));
		} 
		else 
		{
			textures_[i] = nullptr;
		}
	}
	delete parser;
}

/*
 */
GLuint Material::getBlendFactor(const char *factor) 
{
	if(!strcmp(factor,"ZERO")) return GL_ZERO;
	if(!strcmp(factor,"ONE")) return GL_ONE;
	if(!strcmp(factor,"SRC_COLOR")) return GL_SRC_COLOR;
	if(!strcmp(factor,"ONE_MINUS_SRC_COLOR")) return GL_ONE_MINUS_SRC_COLOR;
	if(!strcmp(factor,"SRC_ALPHA")) return GL_SRC_ALPHA;
	if(!strcmp(factor,"ONE_MINUS_SRC_ALPHA")) return GL_ONE_MINUS_SRC_ALPHA;
	if(!strcmp(factor,"ALPHA")) return GL_DST_ALPHA;
	if(!strcmp(factor,"ONE_MINUS_DST_ALPHA")) return GL_ONE_MINUS_DST_ALPHA;
	if(!strcmp(factor,"DST_COLOR")) return GL_DST_COLOR;
	if(!strcmp(factor,"ONE_MINUS_DST_COLOR")) return GL_ONE_MINUS_DST_COLOR;
	fprintf(stderr,"Material::getBlendFactor() unknown blend_ factor \"%s\"\n",factor);
	return 0;
}

/*
 */
GLuint Material::getAlphaFunc(const char *func) 
{
	if(!strcmp(func,"NEVER")) return GL_NEVER;
	if(!strcmp(func,"LESS")) return GL_LESS;
	if(!strcmp(func,"EQUAL")) return GL_EQUAL;
	if(!strcmp(func,"LEQUAL")) return GL_LEQUAL;
	if(!strcmp(func,"GREATER")) return GL_GREATER;
	if(!strcmp(func,"NOTEQUAL")) return GL_NOTEQUAL;
	if(!strcmp(func,"GEQUAL")) return GL_GEQUAL;
	if(!strcmp(func,"ALWAYS")) return GL_ALWAYS;
	fprintf(stderr,"Material::getAlphaFunc() unknown alpha function \"%s\"\n",func);
	return 0;
}

/*****************************************************************************/
/*                                                                           */
/* enable/disable/bind                                                       */
/*                                                                           */
/*****************************************************************************/

/*
 */
int Material::enable() 
{
	if(alpha_test_) 
	{
		if(!old_material_ || (old_material_ && old_material_->alpha_test_ == 0)) 
		{
			glDisable(GL_CULL_FACE);
			glEnable(GL_ALPHA_TEST);
		}
	} 
	else 
	{
		if(old_material_ && old_material_->alpha_test_ == 1) 
		{
			glEnable(GL_CULL_FACE);
			glDisable(GL_ALPHA_TEST);
		}
	}
	if(Core::visible_lights_.size()) 
	{
		if(light_shader_) 
		{
			if(Core::curr_light_ && Core::curr_light_->material_ && Core::curr_light_->material_->light_shader_) 
			{
				Core::curr_light_->material_->light_shader_->enable();
			} 
			else 
			{
				light_shader_->enable();
			}
			return 1;
		}
	} 
	else 
	{
		if(ambient_shader_) 
		{
			ambient_shader_->enable();
			return 1;
		}
	}
	return 0;
}

/*
 */
void Material::disable() 
{
	if(alpha_test_) 
	{
		glEnable(GL_CULL_FACE);
		glDisable(GL_ALPHA_TEST);
	}
	if(Shader::old_shader)
	{
		Shader::old_shader->disable();
	}
	glColor4f(1,1,1,1);
	old_material_ = nullptr;
}

/*
 */
void Material::bind() 
{
	if(old_material_ != this) 
	{
		if(blend_) glBlendFunc(sfactor_,dfactor_);
		if(alpha_test_) glAlphaFunc(alpha_func_, alpha_ref_);
		/*for(int i = 0; i < Shader::NUM_PARAMETERS; i++)
		{
		Shader::setParameter(i,parameters_[i]);
		}*/
		for(int i = 0; i < Shader::NUM_TEXTURES; i++)
		{
			bindTexture(i,textures_[i]);
		}
		old_material_ = this;
	}
	if(Core::visible_lights_.size()) 
	{
		if(light_shader_) 
		{
			if(Core::curr_light_ && Core::curr_light_->material_ && Core::curr_light_->material_->light_shader_) 
			{
				for(int i = 0; i < Shader::NUM_TEXTURES; i++) 
				{
					if(Core::curr_light_->material_->textures_[i]) bindTexture(i,Core::curr_light_->material_->textures_[i]);
				}
				Core::curr_light_->material_->light_shader_->bind();
			} 
			else 
			{
				light_shader_->bind();
			}
		}
	} 
	else 
	{
		if(ambient_shader_) ambient_shader_->bind();
	}
}

/*
 */
void Material::bindTexture(int unit,Texture *texture) 
{
	if(Core::visible_lights_.size()) 
	{
		if(Core::curr_light_ && Core::curr_light_->material_ && Core::curr_light_->material_->light_shader_) 
		{
			Core::curr_light_->material_->light_shader_->bindTexture(unit,texture);
		} 
		else if(light_shader_) 
		{
			light_shader_->bindTexture(unit,texture);
		}
	} 
	else 
	{
		if(ambient_shader_) ambient_shader_->bindTexture(unit,texture);
	}
}
