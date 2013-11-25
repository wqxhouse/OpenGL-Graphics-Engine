#pragma once
#include <vector>

#include "BSPTree.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Material.h"

class Object
{
	Object(int type);
	virtual ~Object();

	virtual void update(float spf);	// update function
	void updatePos(const Vector3 &p);		// update position

	int bindMaterial(const char *name, Material *material);

	enum 
	{
		RENDER_ALL = 0,
		RENDER_OPACITY,
		RENDER_TRANSPARENT
	};

	virtual int render(int t = RENDER_ALL,int s = -1) = 0;

	virtual void findSilhouette(const Vector4 &light,int s = -1) = 0;
	virtual int getNumIntersections(const Vector3 &line0,const Vector3 &line1,int s = -1) = 0;
	virtual int renderShadowVolume(int s = -1)  = 0;

	virtual int intersection(const Vector3 &line0,const Vector3 &line1, Vector3 *point,Vector3 *normal, int s = -1) = 0;

	virtual int getNumSurfaces() = 0;
	virtual const char *getSurfaceName(int s) = 0;
	virtual int getSurface(const char *name) = 0;

	virtual const Vector3 &getMin(int s = -1) = 0;
	virtual const Vector3 &getMax(int s = -1) = 0;
	virtual const Vector3 &getCenter(int s = -1) = 0;
	virtual float getRadius(int s = -1) = 0;

	//void setRigidBody(RigidBody *rigidbody);

	void setShadows(int shadows);

	virtual void set_position(const Vector3 &p);	// set position
	virtual void set_transform(const Matrix4 &m);	// set transformation

	void enable();				// enable transformation
	void disable();				// disable

	enum {
		OBJECT_MESH = 0,
		OBJECT_SKINNEDMESH,
		OBJECT_PARTICLES
	};

	int type;					// type of the object

	Vector3 pos;				// position of the object
	//TODO: potential expansion of the position to allow bezier path moving

	//RigidBody *rigidbody;		// rigidbody dynamic

	int is_identity;
	Matrix4 transform;
	Matrix4 itransform;

	Matrix4 prev_modelview;			// save old matrixes
	Matrix4 prev_imodelview;
	Matrix4 prev_transform;
	Matrix4 prev_itransform;

	std::vector<Material*> materials_;		// all materials

	//TODO: add transparency property

	int shadows_;

	float time_;					// object time
	int frame_;
};