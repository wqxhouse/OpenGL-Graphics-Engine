#pragma once
#include <string>
#include <vector>

#include "core.h"
#include "BSPTree.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Material.h"
#include "RigidBody.h"
#include "Bounds.h"
#include "Position.h"

class Object
{
public:
	enum 
	{
		OBJ_GEOMETRY = 0,
		OBJ_PARTICLES,
	};

	Object(int type);
	virtual ~Object();

	virtual void update(float spf);	// update function
	void updatePos(const Vector3 &p);		// update position

	int bindMaterial(const char *name, Material *material);

	virtual int render(int surface_id = -1) = 0;
	virtual int getIntersection(const Vector3 &l0,
		const Vector3 &l1, 
		Vector3 *point,
		Vector3 *normal, 
		int surface_id = -1) = 0;

	virtual int getNumSurfaces() = 0;
	virtual const char* getSurfaceName(int surface_id) = 0;
	virtual int getSurface(const char *name) = 0;

	//bbox
	virtual const Vector3 getMin(int surface_id = -1) = 0;
	virtual const Vector3 getMax(int surface_id = -1) = 0;

	//bsphere
	virtual const Vector3 getCenter(int surface_id = -1) = 0;
	virtual float getRadius(int surface_id = -1) = 0;

	void setRigidBody(RigidBody *rigidbody);

	virtual void set_position(const Vector3 &p);	// set position
	virtual void set_transform(const Matrix4 &m);	// set transformation

	void enable();				// enable transformation
	void disable();				// disable
	void setShadows(bool b_shadows);

	int type_;					// type of the object

	Position pos_;				// position of the object
	//TODO: potential expansion of the position to allow bezier path moving

	RigidBody *rigidbody_;		// rigidbody dynamic

	int is_identity_;
	Matrix4 transform_;
	Matrix4 itransform_;

	Matrix4 prev_modelview_;			// save old matrixes
	Matrix4 prev_imodelview_;
	Matrix4 prev_transform_;
	Matrix4 prev_itransform_;

	std::vector<Material*> materials_;		// all materials

	bool shadows_;

	float time_;					// object time
	int frame_;

	BSphere bsphere_;
	BBox bbox_;
};