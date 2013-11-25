#include "core.h"
#include "glext.h"
#include "Light.h"
#include "BSPTree.h"
#include "Fog.h"
#include "Frustum.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Camera.h"

// screen
int Core::win_width_;
int Core::win_height_;

Camera Core::camera_;
Frustum *Core::frustum_;

BSPTree *Core::bsp_;

std::vector<Object*> Core::objects_;

std::vector<Light*> Core::lights_;
std::vector<Light*> Core::visible_lights_;
Light *Core::curr_light_;

std::vector<Fog*> Core::fogs_;
std::vector<Fog*> Core::visible_fogs_;
Fog *Core::curr_fog_;

//TODO: occlusion test
//TODO: shadow volume

// current states
float Core::time_;
Vector4 Core::light_;
Vector4 Core::light_color_;
Vector4 Core::fog_color_;
std::vector<int> Core::viewport_;
Matrix4 Core::projection_;
Matrix4 Core::modelview_;
Matrix4 Core::imodelview_;
Matrix4 Core::transform_;
Matrix4 Core::itransform_;

//second per frame
float Core::spf_;

// current frame
int Core::curr_frame_;

bool Core::render_wires_;
bool Core::scissor_test_;
bool Core::render_shadow_;
bool Core::render_fog_;

std::vector<char*> Core::filepaths_;

int Core::texture_filter_;

std::unordered_map<std::string, Shader*>   Core::shaders_;
std::unordered_map<std::string, Texture*>  Core::textures_;
std::unordered_map<std::string, Material*> Core::materials_;
std::unordered_map<std::string, Mesh*>     Core::meshes_;

int Core::init(int win_width, int win_height)
{
#ifdef _WIN32
	glext_init();
#endif
		
	win_width_  = win_width;
	win_height_ = win_height;

	frustum_ = new Frustum();
	bsp_     = new BSPTree();

	curr_light_  = new Light();
	curr_fog_    = new Fog();

	viewport_.resize(4);
	render_wires_ = false;
	render_fog_   = true;
	render_shadow_= true;
	scissor_test_ = true;
	
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

	for(int i = 0; i < fogs_.size(); i++) 
	{
		delete fogs_[i];
	}

	if(bsp_) delete bsp_;
	bsp_ = nullptr;

	std::unordered_map<std::string, Shader*>::iterator shaders_it;
	for(shaders_it = shaders_.begin(); shaders_it != shaders_.end(); shaders_it++) 
	{
		delete shaders_it->second;
	}
	shaders_.clear();

	std::unordered_map<std::string, Texture*>::iterator textures_it;
	for(textures_it = textures_.begin(); textures_it != textures_.end(); textures_it++) 
	{
		delete textures_it->second;
	}
	textures_.clear();

	std::unordered_map<std::string,Material*>::iterator materials_it;
	for(materials_it = materials_.begin(); materials_it != materials_.end(); materials_it++)
	{
		delete materials_it->second;
	}
	materials_.clear();

	std::unordered_map<std::string, Mesh*>::iterator meshes_it;
	for(meshes_it = meshes_.begin(); meshes_it != meshes_.end(); meshes_it++)
	{
		delete meshes_it->second;
	}
	meshes_.clear();
}

