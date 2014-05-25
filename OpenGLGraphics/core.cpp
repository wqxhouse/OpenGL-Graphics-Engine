#include "core.h"
#include <assert.h>

#include "Light.h"
#include "BSPTree.h"
#include "Fog.h"
#include "Frustum.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Camera.h"
#include "Object.h"
#include "WorldFile.h"
#include "Texture.h"
#include "MeshVBO.h"
#include "MeshFileObj.h"
#include "OGeometry.h"
#include <glut/glut.h>

// screen
int Core::win_width_;
int Core::win_height_;

Position Core::camera_(Vector3(0, 0, 0));
Frustum *Core::frustum_;

bool Core::useShadowBit = false;

BSPTree *Core::bsp_;

std::vector<Object*> Core::objects_;

std::vector<Light*> Core::lights_;
std::vector<Light*> Core::visible_lights_;
Light *Core::curr_light_;

GLuint Core::o_query_id_;
bool Core::support_occlusion_;

int Core::num_triangles_;
std::vector<char*> Core::defines_;
//TODO: shadow volume

// current states
float Core::time_;
Vector4 Core::light_pos_;
Vector4 Core::light_color_;
std::vector<int> Core::viewport_;
Matrix4 Core::projection_;
Matrix4 Core::modelview_;
Matrix4 Core::imodelview_;
Matrix4 Core::transform_;
Matrix4 Core::itransform_;

//post
Texture *Core::texture_;
Material *Core::material_;

//shadow maps
Shader *Core::omni_shadow_shader_;
GLuint Core::cubeTex_;
GLuint Core::cubeDepthTex_;
GLuint Core::cubeFBOs_[6];
GLuint Core::currentSideTex_;
GLuint Core::currentSideDepthTex_;
GLuint Core::toCurrentSideFBO_;

//second per frame
float Core::spf_;

// current frame
int Core::curr_frame_;

bool Core::render_wires_;
bool Core::scissor_test_;
bool Core::render_shadow_;

std::vector<char*> Core::filepaths_;

int Core::texture_filter_ = Texture::LINEAR_MIPMAP_LINEAR;

std::map<std::string, Shader*>   Core::shaders_;
std::map<std::string, Texture*>  Core::textures_;
std::map<std::string, Material*> Core::materials_;
std::map<std::string, Mesh*>     Core::meshes_;


GLuint g_fbo;
GLuint g_dbgtex;

void Core::genDebugFBO(GLuint *dbgFBO, GLuint tex)
{
	GLenum FBOstatus;

	// Try to use a texture depth component
	glGenTextures(1, &g_dbgtex);
	glBindTexture(GL_TEXTURE_2D, g_dbgtex);

	// GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Remove artefact on the edges of the shadowmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

	//glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );



	// No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available 
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 512, 512, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
	glBindTexture(GL_TEXTURE_2D, 0);

	// create a framebuffer object
	glGenFramebuffers(1, &g_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);

	// Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	// attach the texture to FBO depth attachment point
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D, g_dbgtex, 0);

	// check FBO status
	FBOstatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(FBOstatus != GL_FRAMEBUFFER_COMPLETE)
		printf("GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n");

	// switch back to window-system-provided framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}




int Core::init(int win_width, int win_height)
{		
	win_width_  = win_width;
	win_height_ = win_height;

	frustum_ = new Frustum();
	bsp_     = nullptr;

	curr_light_  = nullptr;

	render_wires_ = false;
	render_shadow_= true;
	scissor_test_ = true;

	viewport_.resize(4);

	//assume my laptop's x4500 is ok with occ query
	support_occlusion_ = true;
	glGenQueriesARB(1, &o_query_id_);
	
	time_ = 0;
	curr_frame_ = 0;

	printf("Core::init() OK...\n");

	return 1;
}

void Core::cleanUp()
{
	for(int i = 0; i < objects_.size(); i++) 
	{
		delete objects_[i];
	}

	for(int i = 0; i < lights_.size(); i++) 
	{
		delete lights_[i];
	}

	if(bsp_) delete bsp_;
	bsp_ = nullptr;

	std::map<std::string, Shader*>::iterator shaders_it;
	for(shaders_it = shaders_.begin(); shaders_it != shaders_.end(); shaders_it++) 
	{
		delete shaders_it->second;
	}
	shaders_.clear();

	std::map<std::string, Texture*>::iterator textures_it;
	for(textures_it = textures_.begin(); textures_it != textures_.end(); textures_it++) 
	{
		delete textures_it->second;
	}
	textures_.clear();

	std::map<std::string,Material*>::iterator materials_it;
	for(materials_it = materials_.begin(); materials_it != materials_.end(); materials_it++)
	{
		delete materials_it->second;
	}
	materials_.clear();

	std::map<std::string, Mesh*>::iterator meshes_it;
	for(meshes_it = meshes_.begin(); meshes_it != meshes_.end(); meshes_it++)
	{
		delete meshes_it->second;
	}
	meshes_.clear();
	
	if(texture_)
	{
		delete texture_;
	}
	if(material_)
	{
		delete material_;
	}
}


/*****************************************************************************/
/*                                                                           */
/* loaders                                                                   */
/*                                                                           */
/*****************************************************************************/

/* support '|' as "OR" operation, '?' as ANY symbol and '*' as ANY string
 */
int Core::match(const char *mask,const char *name) {
	char *m = (char*)mask;
	char *n = (char*)name;
	int match = 1;
	while(1) {
		if((match && *m == '*')|| *m == '\0') break;
		if(*m == '|') {
			if(match) break;
			m++;
			n = (char*)name;
			match = 1;
		} else {
			if(*m != '?' && *m != *n) match = 0;
			if(*n) n++;
			m++;
		}
	}
	return match;
}

/*
 */
void Core::AddDirectoryPath(const char *path) 
{
	char *s = (char*)path;
	while(1) {
		char *p = new char[strlen(s) + 1];
		char *d = p;
		while(*s != '\0' && *s != ',') *d++ = *s++;
		*d = '\0';
		Core::filepaths_.push_back(p);
		if(*s == '\0') break;
		else s++;
	}
}

const char *Core::FindFile(const char *name) 
{
	static char buf[1024];
	for(char *s = (char*)name; *s != '\0'; s++) {
		if(*s == '%' && *(s + 1) == 's') {
			static char complex_name[1024];
			sprintf(complex_name,name,"px");
			s = (char*)FindFile(complex_name);
			if(strcmp(s,name)) {
				for(s = buf + strlen(buf); s > buf; s--) {
					if(*s == 'x' && *(s - 1) == 'p') {
						*s = 's';
						*(s - 1) = '%';
						break;
					}
				}
				return buf;	// hm
			}
			break;
		}
	}
	for(int i = (int)filepaths_.size() - 1; i >= -1; i--) {
		if(i == -1) sprintf(buf,"%s",name);
		else sprintf(buf,"%s%s",filepaths_[i],name);
		FILE *file = fopen(buf,"rb");
		if(file) {
			fclose(file);
			return buf;
		}
	}
	fprintf(stderr,"Core::FindFile(): can`t find \"%s\" file\n",name);
	return name;
}

/*
 */
void Core::define(const char *define) 
{
	int i;
	for(i = 0; i < (int)defines_.size(); i++) if(!strcmp(defines_[i],define)) break;
	if(i == (int)defines_.size()) {
		char *d = new char[strlen(define) + 1];
		strcpy(d,define);
		defines_.push_back(d);
	}
}

void Core::undef(const char *define) 
{
	for(int i = 0; i < (int)defines_.size(); i++) {
		if(!strcmp(defines_[i],define)) {
			delete defines_[i];
			defines_.erase(defines_.begin() + i);
		}
	}
}

int Core::isDefine(const char *define) {
	for(int i = 0; i < (int)defines_.size(); i++) if(!strcmp(defines_[i],define)) return 1;
	return 0;
}

/*
 */
void Core::LoadScene(const char *name) 
{
	cleanUp();
	WorldFile::load(FindFile(name));
	/*if(ex) extern_load(extern_load_data);
	console->printf("load \"%s\" ok\n",name);*/

	/*texture_  = new Texture(win_width_, win_height_, Texture::TEXTURE_2D,Texture::RGB | Texture::CLAMP | Texture::LINEAR);
	material_ = LoadMaterial("screen.mat");*/

	//load shadow map shader
	omni_shadow_shader_ = LoadShader("omni_shadow.shader");

	cubeTex_ = Texture::genCubeTexture(512);
	cubeDepthTex_ = Texture::genCubeDepthTexture(512);

	assert(cubeTex_ != -1 && cubeDepthTex_ != -1 && "cube tex failed\n");
	genCubeFBOs(cubeFBOs_, cubeTex_, cubeDepthTex_);

	genDebugFBO(&g_fbo, g_dbgtex);
} 

/*
 */
Shader *Core::LoadShader(const char *name) 
{
	std::map<std::string,Shader*>::iterator it = shaders_.find(name);
	if(it == shaders_.end()) 
	{
		Shader *shader = new Shader(FindFile(name));
		shaders_[name] = shader;
		return shader;
	}
	return it->second;
}

Texture *Core::LoadTexture(const char *name,GLuint target,int flag) 
{
	std::map<std::string,Texture*>::iterator it = textures_.find(name);
	if(it == textures_.end()) 
	{
		Texture *texture = new Texture(FindFile(name), target, flag);
		textures_[name] = texture;
		return texture;
	}
	return it->second;
}

Material *Core::LoadMaterial(const char *name) 
{
	std::map<std::string,Material*>::iterator it = materials_.find(name);
	if(it == materials_.end())
	{
		Material *material = new Material(FindFile(name));
		materials_[name] = material;
		return material;
	}
	return it->second;
}

Mesh *Core::LoadGeometry(const char *name) 
{
	std::map<std::string,Mesh*>::iterator it = meshes_.find(name);
	if(it == meshes_.end()) 
	{
		MeshFileOBJ meshobj;
		meshobj.load(FindFile(name));
		
		Mesh *mesh = new MeshVBO(meshobj);
		meshes_[name] = mesh;
		return mesh;
	}
	return it->second;
}

void Core::genCubeFBOs(GLuint *cubeFBOs, GLuint cubeTex, GLuint cubeDepthTex)
{
	glGenFramebuffersEXT(6, cubeFBOs);
	for(int i=0; i<6; i++) 
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER, cubeFBOs[i]);
		glFramebufferTexture2DEXT (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, cubeTex, 0);
		glFramebufferTexture2DEXT (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, cubeDepthTex, 0);
		GLenum result = glCheckFramebufferStatus (GL_FRAMEBUFFER);
		if (GL_FRAMEBUFFER_COMPLETE != result) 
		{
			printf ("ERROR: Framebuffer is not complete.\n");
		}
	}
	glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
}


void Core::AddObject(Object *object) 
{
	if(objects_.size() > MAX_NUM_OBJECTS)
	{
		assert(false && "exceed obj maximum");
	}

	object->update(0.0);
	objects_.push_back(object);
}

void Core::RemoveObject(Object *object) 
{
	if(object->pos_.in_sector_id_ != -1)
	{
		BSPTree::sectors_[object->pos_.in_sector_id_].removeObject(object);
	}

	for(int i = 0; i < objects_.size(); i++) 
	{
		if(objects_[i] == object) 
		{
			objects_.erase(objects_.begin() + i);
			return;
		}
	}
}

/*
 */
void Core::AddLight(Light *light) 
{
	if(lights_.size() > MAX_NUM_OBJECTS)
	{
		assert(false && "exceed maximum num of lights");
	}

	light->update(0.0);
	lights_.push_back(light);
}

void Core::RemoveLight(Light *light) 
{
	for(int i = 0; i < lights_.size(); i++) 
	{
		if(lights_[i] == light) 
		{
			lights_.erase(lights_.begin() + i);
			return;
		}
	}
}


/*****************************************************************************/
/*                                                                           */
/* update                                                                    */
/*                                                                           */
/*****************************************************************************/

void Core::Update(float spf) 
{
	Core::spf_ = spf;
	
	// new frame
	curr_frame_++;

	// update time
	time_ += spf;
	
	// update lights
	for(int i = 0; i < visible_lights_.size(); i++) 
	{
		visible_lights_[i]->update(spf);
	}
	
	// update objects
	Object *objects[Sector::NUM_OBJECTS];
	for(int i = 0; i < BSPTree::visible_sectors_.size(); i++) 
	{
		Sector *s = BSPTree::visible_sectors_[i];
		for(int j = 0; j < s->objects_.size(); j++)
		{
			objects[j] = s->objects_[j];
		}

		int num_objects = s->objects_.size();
		for(int j = 0; j < num_objects; j++)
		{
			Object *o = objects[j];
			if(o->frame_ == -Core::curr_frame_)
			{
				continue;
			}
			 
			o->update(spf);
			o->frame_ = -Core::curr_frame_;
		}
	}
}

void Core::set_shadow_matrix_uniform(GLuint shaderProgram, int dir, const Vector3 &lightPos)
{
	Matrix4 mat, view, tmp;

	Matrix4 tmpPersp;
	tmpPersp.setPerspectiveMat(90.0f, 1.0f, 0.5f, 100.0f);
	mat = mat.multiplyMat( tmpPersp);
	switch(dir) 
	{
	case 0:
		// +X
		tmp.setLookAtMat(lightPos, lightPos+Vector3(+1,+0,0), Vector3(0,0,-1));
		view = tmp;
		mat=  mat.multiplyMat(tmp);
		break;
	case 1:
		// -X
		tmp.setLookAtMat(lightPos, lightPos+Vector3(-1,+0,0), Vector3(0,0,-1));
		view = tmp;
		mat=  mat.multiplyMat(tmp);
		break;
	case 2:
		// +Y
		tmp.setLookAtMat(lightPos, lightPos+Vector3(0,+1,0), Vector3(0,0,-1));
		view = tmp;
		mat=  mat.multiplyMat(tmp);
		break;
	case 3:
		// -Y
		tmp.setLookAtMat(lightPos, lightPos+Vector3(0,-1,0), Vector3(0,0,-1));
		view = tmp;
		mat=  mat.multiplyMat(tmp);
		break;
	case 4:
		// +Z
		tmp.setLookAtMat(lightPos, lightPos+Vector3(0,0,+1), Vector3(0,-1,0));
		view = tmp;
		mat=  mat.multiplyMat(tmp);
		break;
	case 5:
		// -Z
		// Works
		tmp.setLookAtMat(lightPos, lightPos+Vector3(0,0,-1),Vector3(0,-1,0));
		view = tmp;
		mat=  mat.multiplyMat(tmp);
		break;
	default:
		// Not needed
		return;
		break;
	}

	//////////////////////////////////////////////////////////////////////////
	//switch(dir) 
	//{
	//case 0:
	//	// +X
	//	tmp.setLookAtMat(lightPos, lightPos+Vector3(+1,+0,0), Vector3(0,-1,0));
	//	view = tmp;
	//	mat=  mat.multiplyMat(tmp);
	//	break;
	//case 1:
	//	// -X
	//	tmp.setLookAtMat(lightPos, lightPos+Vector3(-1,+0,0), Vector3(0,-1,0));
	//	view = tmp;
	//	mat=  mat.multiplyMat(tmp);
	//	break;
	//case 2:
	//	// +Y
	//	tmp.setLookAtMat(lightPos, lightPos+Vector3(0,+1,0), Vector3(0,0,-1));
	//	view = tmp;
	//	mat=  mat.multiplyMat(tmp);
	//	break;
	//case 3:
	//	// -Y
	//	tmp.setLookAtMat(lightPos, lightPos+Vector3(0,-1,0), Vector3(0,0,-1));
	//	view = tmp;
	//	mat=  mat.multiplyMat(tmp);
	//	break;
	//case 4:
	//	// +Z
	//	tmp.setLookAtMat(lightPos, lightPos+Vector3(0,0,+1), Vector3(0,-1,0));
	//	view = tmp;
	//	mat=  mat.multiplyMat(tmp);
	//	break;
	//case 5:
	//	// -Z
	//	// Works
	//	tmp.setLookAtMat(lightPos, lightPos+Vector3(0,0,-1),Vector3(0,-1,0));
	//	view = tmp;
	//	mat=  mat.multiplyMat(tmp);
	//	break;
	//default:
	//	// Not needed
	//	return;
	//	break;
	//}
	omni_shadow_shader_->setParameterFloat("cameraToShadowView", view.getPointer(), 16);
	omni_shadow_shader_->setParameterFloat("cameraToShadowProjector", mat.getPointer(), 16);
}

void Core::render_omni_shadow(const Light *light)
{
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	////glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	////glActiveTextureARB(GL_TEXTURE0);
	////glBindTexture(GL_TEXTURE_2D, g_dbgtex);

	//
	////glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	//omni_shadow_shader_->enable();
	//omni_shadow_shader_->bind();


	//	glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
	//	glViewport(0, 0, 512, 512); //shadow map size

	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//	set_shadow_matrix_uniform(omni_shadow_shader_->getProgramID(), 5, light->pos());

	//	//draw visible objects
	//	for(int i = 0; i < BSPTree::visible_sectors_.size(); i++) 
	//	{
	//		Sector *s = BSPTree::visible_sectors_[i];
	//		Portal *p = (s->frame_ == Core::curr_frame_) ? s->portal_ : nullptr;
	//		
	//		for(int j = 0; j < s->visible_objects_.size(); j++) 
	//		{
	//			Object *o = s->visible_objects_[j];
	//			if((o->pos_.getPosCoord() + o->getCenter() - light_pos_.getVector3()).getLength() < o->getRadius() + light->radius()) 
	//			{
	//				num_triangles_ += o->render(-1, true);
	//			}
	//		}
	//	}

	//// Reset
	//omni_shadow_shader_->disable();
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//glBindTexture(GL_TEXTURE_2D, 0);
	//glViewport(0, 0, win_width_, win_height_);
	////glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	///*glDisable(GL_DEPTH_TEST);
	//glDisable(GL_CULL_FACE);*/

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////





	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	//glActiveTextureARB(GL_TEXTURE7);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, cubeDepthTex_);

	glViewport(0, 0, 512, 512); //shadow map size
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	omni_shadow_shader_->enable();
	omni_shadow_shader_->bind();

	for(int i=0; i<6; ++i) 
	{
		glBindFramebuffer(GL_FRAMEBUFFER, cubeFBOs_[i]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		set_shadow_matrix_uniform(omni_shadow_shader_->getProgramID(),i, light->pos());

		//draw visible objects
		for(int i = 0; i < BSPTree::visible_sectors_.size(); i++) 
		{
			Sector *s = BSPTree::visible_sectors_[i];
			Portal *p = (s->frame_ == Core::curr_frame_) ? s->portal_ : nullptr;
			
			for(int j = 0; j < s->visible_objects_.size(); j++) 
			{
				Object *o = s->visible_objects_[j];

				if((o->pos_.getPosCoord() + o->getCenter() - light_pos_.getVector3()).getLength() < o->getRadius() + light->radius()) 
				{
					num_triangles_ += o->render(-1, true);
				}
			}
		}
	}

	// Reset
	omni_shadow_shader_->disable();
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glViewport(0, 0, win_width_, win_height_);
	//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	/*glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);*/
}


void Core::render_light() 
{
	
	if(support_occlusion_) 
	{
		glDisable(GL_CULL_FACE);
		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	}
	
	// find visible lights
	for(int i = 0; i < lights_.size(); i++) 
	{
		Light *l = lights_[i];

		for(int j = 0; j < l->pos_.sectors_.size(); j++) 
		{
			if(BSPTree::sectors_[l->pos_.sectors_[j]].frame_ != curr_frame_)
			{
				continue;
			}
			
			if(frustum_->inside(l->pos(), l->radius()) == 0)
			{
				continue;
			}
			
			if(support_occlusion_ && (l->pos() - camera_.getPosCoord()).getLength() > l->radius()) 
			{
				glPushMatrix();
				glTranslatef(
					l->pos_.getPosCoord()['x'], 
					l->pos_.getPosCoord()['y'], 
					l->pos_.getPosCoord()['z']);

				glScalef(l->radius(), l->radius(), l->radius());

				glBeginQueryARB(GL_SAMPLES_PASSED_ARB, o_query_id_);
				glutSolidSphere(1.0f, 16, 8);
				glEndQueryARB(GL_SAMPLES_PASSED_ARB);
				glPopMatrix();
				
				GLuint samples;
				glGetQueryObjectuivARB(o_query_id_, GL_QUERY_RESULT_ARB, &samples);
				//printf("samples: %d\n", samples);

				if(samples == 0) continue;
			}
			visible_lights_.push_back(l);
			break;
		}
	}
	if(support_occlusion_) 
	{
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
		glEnable(GL_CULL_FACE);
	}

	if(scissor_test_) glEnable(GL_SCISSOR_TEST);
	
	//////////////////////////////////////////////////////////////////////////
	// render lights
	for(int i = 0; i < visible_lights_.size(); i++) 
	{
		Light *l = visible_lights_[i];
		curr_light_ = l;
		
		light_pos_ = Vector4(l->pos_.getPosCoord(), 1.0 / (l->radius() * l->radius()));
		light_color_ = l->color_;

		// scissor
		int scissor[4];
		l->getScissor(scissor);
		if(scissor_test_) glScissor(scissor[0],scissor[1],scissor[2],scissor[3]);
		
		//////////////////////////////////////////////////////////////////////////
		//depth pass
		if(useShadowBit)
		{
			render_omni_shadow(l);
		}
		
		//display debug texture
	/*		glUseProgramObjectARB(0);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-win_width_/2,win_width_/2,-win_height_/2,win_height_/2,1,20);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glColor4f(1,1,1,1);
		glActiveTextureARB(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, g_dbgtex);
		glEnable(GL_TEXTURE_2D);
		glTranslated(0,0,-1);
		glBegin(GL_QUADS);
		glTexCoord2d(0,0);glVertex3f(0,0,0);
		glTexCoord2d(1,0);glVertex3f(win_width_/2,0,0);
		glTexCoord2d(1,1);glVertex3f(win_width_/2,win_height_/2,0);
		glTexCoord2d(0,1);glVertex3f(0,win_height_/2,0);


		glEnd();
		glDisable(GL_TEXTURE_2D);*/

		
		//////////////////////////////////////////////////////////////////////////
		//normal pass
		glDepthFunc(GL_EQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);

		for(int i = 0; i < BSPTree::visible_sectors_.size(); i++) 
		{
			Sector *s = BSPTree::visible_sectors_[i];
			Portal *p = (s->frame_ == Core::curr_frame_) ? s->portal_ : nullptr;
			if(p) 
			{
				int portal_scissor[4];
				p->getScissor(portal_scissor);
				portal_scissor[2] += portal_scissor[0];
				portal_scissor[3] += portal_scissor[1];
				if(portal_scissor[0] < scissor[0]) portal_scissor[0] = scissor[0];
				if(portal_scissor[1] < scissor[1]) portal_scissor[1] = scissor[1];
				if(portal_scissor[2] > scissor[0] + scissor[2]) portal_scissor[2] = scissor[0] + scissor[2];
				if(portal_scissor[3] > scissor[1] + scissor[3]) portal_scissor[3] = scissor[1] + scissor[3];
				portal_scissor[2] -= portal_scissor[0];
				portal_scissor[3] -= portal_scissor[1];
				if(portal_scissor[2] < 0 || portal_scissor[3] < 0) continue;
				if(scissor_test_) glScissor(portal_scissor[0],portal_scissor[1],portal_scissor[2],portal_scissor[3]);
			} else { 
				if(scissor_test_) glScissor(scissor[0],scissor[1],scissor[2],scissor[3]);
			}
			for(int j = 0; j < s->visible_objects_.size(); j++) 
			{
				Object *o = s->visible_objects_[j];

				if((o->pos_.getPosCoord() + o->getCenter() - light_pos_.getVector3()).getLength() < o->getRadius() + l->radius()) 
				{
					if(useShadowBit)
					{
						glActiveTextureARB(GL_TEXTURE7);
						glBindTexture(GL_TEXTURE_CUBE_MAP, cubeDepthTex_);

						num_triangles_ += o->render();
						glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
					}
					else
					{
						num_triangles_ += o->render();
					}
				}
			}
		}
		
		if(scissor_test_) glScissor(viewport_[0],viewport_[1],viewport_[2],viewport_[3]);
	
		// disable material
		if(Material::old_material_) Material::old_material_->disable();
		
		glDisable(GL_BLEND);
		glDepthFunc(GL_LEQUAL);
		
	}
	curr_light_ = nullptr;
	
	if(scissor_test_) 
	{
		glScissor(viewport_[0],viewport_[1],viewport_[2],viewport_[3]);
		glDisable(GL_SCISSOR_TEST);
	}
	
}

/*
 */
void Core::RenderScene(float spf) 
{
	//printf("FPS: %.2f\r", 1.0f / spf);
	// get matrixes
	float projPOD[16];
	float modelvPOD[16];
	glGetFloatv(GL_PROJECTION_MATRIX,projPOD);
	glGetFloatv(GL_MODELVIEW_MATRIX,modelvPOD);
	projection_ = Matrix4(projPOD);
	modelview_  = Matrix4(modelvPOD);

	imodelview_ = modelview_.getInverse();
	
	transform_.loadIdentity();
	itransform_.loadIdentity();
	
	// get camera
	camera_.setPosition(imodelview_.multiplyVec3(Vector3(0,0,0)));

	// set frustum
	frustum_->set(projection_.multiplyMat(modelview_));
	
	// global triangle counter
	num_triangles_ = 0;
	
	// get viewport
	glGetIntegerv(GL_VIEWPORT, &viewport_[0]);

	// ambient pass
	// clear depth buffer
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LESS);
	
	// number of visible lights
	// zero - ambient shader
	visible_lights_.resize(0);
	
	// render bsp
	if(bsp_) bsp_->render();
	
	// disable material
	if(Material::old_material_)
	{
		Material::old_material_->disable();
	}
	
	// light pass
	render_light();
	
	// disable material
	if(Material::old_material_)
	{
		Material::old_material_->disable();
	}
	
	// wireframe
	if(render_wires_) 
	{
		glColor3f(0, 1, 0);
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
		glEnable(GL_POLYGON_OFFSET_LINE);
		glPolygonOffset(-0.5,0.0);
		for(int i = 0; i < BSPTree::visible_sectors_.size(); i++)
		{
			Sector *s = BSPTree::visible_sectors_[i];
			for(int j = 0; j < s->visible_objects_.size(); j++) 
			{
				Object *o = s->visible_objects_[j];
				num_triangles_ += o->render();
			}
		}
		glDisable(GL_POLYGON_OFFSET_LINE);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	}
	
	glFlush();
}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

/*
 */
Object *Core::GetIntersectObject(const Vector3 &l0,const Vector3 &l1, Vector3 *pt,Vector3 *normal) 
{
	Vector3 p,n;
	*pt = l1;

	Object *object = nullptr;
	for(int i = 0; i < BSPTree::visible_sectors_.size(); i++) 
	{
		Sector *s = BSPTree::visible_sectors_[i];
		for(int j = 0; j < s->visible_objects_.size(); j++) 
		{
			Object *o = s->visible_objects_[j];
			if(o->is_identity_) 
			{
				if(o->getIntersection(l0,*pt,&p,&n)) 
				{
					*pt = p;
					*normal = n;
					object = o;
				}
			} 
			else 
			{
				Vector3 l0 = o->itransform_.multiplyVec3(l0);
				Vector3 l1 = o->itransform_.multiplyVec3(*pt);
				if(o->getIntersection(l0,l1,&p,&n)) 
				{
					*pt = o->transform_.multiplyVec3(p);
					*normal = o->transform_.getRotationPart().multiplyVec3(n);
					object = o;
				}
			}
		}
	}
	return object;
}
