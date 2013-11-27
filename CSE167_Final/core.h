#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <glut/glut.h>


class Object;
class Light;
class Fog;
class Texture;
class Material;
class Frustum;
class BSPTree;
class Camera;
class Vector4;
class Matrix4;
class Shader;
class Mesh;

class Core
{
public:
	static int init(int win_width, int win_height);
	static void cleanUp();

	static void AddDirectoryPath(const char *path);
	static void FindFile(const char *filename);

	static void LoadScene(const char *filename);
	static void LoadShader(const char *shader);
	static void LoadMaterial(const char *material);
	static void LoadGeometry(const char *mesh);

	static void AddObject(Object *obj);
	static void RemoveObject(Object *obj);
	static void AddLight(Light *light);
	static void RemoveLight(Light *light);
	static void AddFog(Fog *fog);
	static void RemoveFog(Fog *fog);

	static void UpdateStates(float spf);
	static void RenderScene(float spf);
	static Object *GetIntersectObject(const Vector3 &l0,const Vector3 &l1, Vector3 *pt, Vector3 *n);

	//static ObjectBase *intersection

	// screen
	static int win_width_;
	static int win_height_;

	static Camera camera_;
	static Frustum *frustum_;



	//// lights
	//static std::vector<Light*> lights_;
	//static std::vector<Light*> visible_lights_;
	//static Light *curr_light_;

	//// fogs
	//static std::vector<Fog*> fogs_;
	//static std::vector<Fog*> visible_fogs_;
	//static Fog *curr_fog_;

	//TODO: occlusion test
	//TODO: shadow volume

	// current state
	static float time_;
	static Vector4 light_;
	static Vector4 light_color_;
	static Vector4 fog_color_;
	static std::vector<int> viewport_;
	static Matrix4 projection_;
	static Matrix4 modelview_;
	static Matrix4 imodelview_;
	static Matrix4 transform_;
	static Matrix4 itransform_;

	//second per frame
	static float spf_;

	// current frame
	static int curr_frame_;

	static bool render_wires_;
	static bool scissor_test_;
	static bool render_shadow_;
	static bool render_fog_;

	static std::vector<char*> filepaths_;

	static int texture_filter_;

	static std::unordered_map<std::string, Shader*>   shaders_;
	static std::unordered_map<std::string, Texture*>  textures_;
	static std::unordered_map<std::string, Material*> materials_;
	static std::unordered_map<std::string, Mesh*>     meshes_;
};