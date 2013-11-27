#pragma once
#include "Object.h"

class Light;
class Object;
class Material;
class Shader;
class Sector;

class ObjectSurface
{
public:

	ObjectSurface();
	ObjectSurface(Object *object, int surface);
	~ObjectSurface();

	ObjectSurface &operator=(const ObjectSurface &s);

	// parameters
	inline int getType() const { return type; }
	inline int getSurface() const { return surface; }
	inline Object *getObject() const { return object; }

	// frame number
	inline void setFrame(int f) { frame = f; }
	inline int getFrame() const { return frame; }

	// update
	void update();
	void updatePosition();

	// resources
	inline int getCastShadow() const;
	inline int getReceiveShadow() const;
	inline int getCastWorldShadow() const;
	inline int getReceiveWorldShadow() const;

	inline int getMinParent() const;
	inline float getMinVisibleDistance() const;
	inline float getMaxFadeDistance() const;

	inline int getMaxParent() const;
	inline float getMaxVisibleDistance() const;
	inline float getMinFadeDistance() const;

	inline int isFaded() const;

	inline void setMaterial(Material *m) { material = m; }
	inline Material *getMaterial() const { return material; }

	Material *getMaterialInherit();
	int isMaterialInherited() const;

	Shader *getShader(int pass);

	// world sector
	WorldSector *getWorldSector() const;

	// compares
	inline int getResource() const { return resource; }
	inline int getOrder(const Vector3 &camera) const;
	inline int getSequence(const Vector3 &camera) const;
	inline float getOpacityDistance(const Vector3 &camera) const;
	inline float getTransparentDistance(const Vector3 &camera) const;

	// transformaion
	inline int isWorldRender() { return is_world_render; }
	inline const Vector4 *getWorldTransform() const { return world_transform; }

	// line intersections
	inline int getIntersection(const Vector3 &p0,const Vector3 &p1) const;
	inline int getIntersection(const Vector3 &p0,const Vector3 &p1,Vector3 &ret_point,Vector4 &ret_plane,int &ret_triangle) const;
	inline int getIntersection(const Vector3 &p0,const Vector3 &p1,Vector3 &ret_point,Vector3 &ret_normal,Vector4 &ret_texcoord) const;

	// bounds
	inline const BBox &getWorldBBox() const { return world_bound_box; }
	inline const BSphere &getWorldBSphere() const { return world_bound_sphere; }

	// chain of surfaces
	inline void setShader(Shader *s) { pass = -1; shader = s; }
	inline Shader *getShader() const { return shader; }

	inline void setPrev(ObjectSurface *p) { prev = p; }
	inline ObjectSurface *getPrev() const { return prev; }

	inline void setNext(ObjectSurface *n) { next = n; }
	inline ObjectSurface *getNext() const { return next; }

	// create
	inline int hasCreate() const { return has_create; }
	inline ObjectSurface *create();

	// render
	inline int isEmptyRender(const Light *light);
	inline int isEmptyRender(const BSphere &bs);
	inline ObjectSurface *render(int pass);
	inline ObjectSurface *render(int pass,const Light *light);
	inline ObjectSurface *render(int pass,const BSphere &bs);

	// render shadow map
	inline int hasShadow() const { return has_shadow; }
	inline ObjectSurface *renderShadow(int pass);
	inline ObjectSurface *renderShadow(int pass,const Light *light);
	inline ObjectSurface *renderShadow(int pass,const BSphere &bs);

	// render velocity
	inline int hasVelocity() const;
	inline ObjectSurface *renderVelocity(int pass);

private:

	int frame;								// frame number

	char type;								// object type
	char pass;								// current pass

	short surface;							// surface number

	bool has_create;			// has create flag
	bool has_shadow;			// has shadow flag
	bool is_world_render;		// world render flag

	Object *object;							// object pointer
	Material *material;						// object material
	int resource;							// object resource

	Shader *shader;							// current shader
	ObjectSurface *prev;					// previous object surface
	ObjectSurface *next;					// next object surface

	Vector4 world_transform[3];				// world transformaion
	BBox world_bound_box;				// world bound box
	BSphere world_bound_sphere;			// world bound sphere
};


