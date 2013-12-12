#pragma once
#include <vector>
#include <string>
#include <map>
#ifdef _WIN32
#include "GLee.h"
#endif

#include "Frustum.h"
#include "Position.h"



class Object;
class Light;
class Fog;
class Texture;
class Material;
class Frustum;
class BSPTree;
class Camera;
class Vector3;
class Vector4;
class Matrix4;
class Shader;
class Mesh;

class Core
{
public:
	static const int MAX_NUM_OBJECTS = 256;

	static int init(int win_width, int win_height);
	static void cleanUp();

	static void AddDirectoryPath(const char *path);
	static const char* FindFile(const char *filename);
	void define(const char *define);
	void undef(const char *define);

	static void      LoadScene(const char *filename);
	static Shader*   LoadShader(const char *shader);
	static Material* LoadMaterial(const char *material);
	static Mesh*     LoadGeometry(const char *mesh);
	static Texture*  LoadTexture(const char *name, GLuint target, int flag);

	static void AddObject(Object *obj);
	static void RemoveObject(Object *obj);
	static void AddLight(Light *light);
	static void RemoveLight(Light *light);
	static void AddFog(Fog *fog);
	static void RemoveFog(Fog *fog);
	static int isDefine(const char *define);

	static void Update(float spf);
	static void RenderScene(float spf);
	static Object *GetIntersectObject(const Vector3 &l0,const Vector3 &l1, Vector3 *pt, Vector3 *normal);

	static int match(const char *mask,const char *name);

	static void render_light();
	static void render_omni_shadow(const Light *light);
	static void genCubeFBOs(GLuint *cubeFBOs, GLuint cubeTex, GLuint cubeDepthTex);
	static void set_shadow_matrix_uniform(GLuint shaderProgram, int dir, const Vector3 &lightPos);


	static void genDebugFBO(GLuint *dbgFBO, GLuint tex);

	static bool support_occlusion_;
	static GLuint o_query_id_;

	// screen
	static int win_width_;
	static int win_height_;
	static int num_triangles_;

	static Position camera_;
	static Frustum *frustum_;

	static BSPTree *bsp_;

	static std::vector<Object*> objects_;

	// lights
	static std::vector<Light*> lights_;
	static std::vector<Light*> visible_lights_;
	static Light *curr_light_;


	//TODO: occlusion test
	//TODO: shadow volume

	// current state
	static float time_;
	static Vector4 light_pos_;
	static Vector4 light_color_;
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
	//static bool render_fog_;

	static std::vector<char*> filepaths_;

	static int texture_filter_;
	static Texture *texture_;
	static Material *material_;

	static std::map<std::string, Shader*>   shaders_;
	static std::map<std::string, Texture*>  textures_;
	static std::map<std::string, Material*> materials_;
	static std::map<std::string, Mesh*>     meshes_;

	static std::vector<char*> defines_;

	//shadow maps
	static Shader *omni_shadow_shader_;
	static GLuint cubeTex_;
	static GLuint cubeDepthTex_;
	static GLuint cubeFBOs_[6];
	static GLuint currentSideTex_;
	static GLuint currentSideDepthTex_;
	static GLuint toCurrentSideFBO_;
	static bool useShadowBit;

};