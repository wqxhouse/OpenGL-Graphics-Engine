#pragma once
#include <vector>
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
#include "BSPTree.h"

class Frustum
{
public:

	Frustum();
	~Frustum();

	void applyCurrProjMVMat();
	void set(const Matrix4 &m);

	void addPortal(const Vector3 &point,const Vector3 *points);
	void removePortal();

	//int inside(const Vector3 &min, const Vector3 &max);
	int inside(const Vector3 &point, float radius);
	int inside(const Vector3 *points, int num);

	int inside_portal(const Portal &p, const Vector3 &point, float radius);
	/*int inside(const Vector3 &light, float light_radius, const Vector3 &center, float radius);
	int inside_all(const Vector3 &min,const Vector3 &max);
	int inside_all(const Vector3 &center,float radius);*/

protected:

	enum 
	{
		PLANE_LEFT = 0, 
		PLANE_RIGHT, 
		PLANE_TOP, 
		PLANE_BOTTOM, 
		PLANE_NEAR, 
		PLANE_FAR
	};

	struct Portal 
	{
		vec4 plane;							// portal plane
		vec4 planes[4];						// clip planes
		vec4 planes_x;						// clip planes x
		vec4 planes_y;						// clip planes y
		vec4 planes_z;						// clip planes z
		vec4 planes_w;						// clip planes w
		vec3 points[4];						// bounding points
	};

	std::vector<Vector4> planes_;

	void extractPlanes(
		std::vector<Vector4> *p_planes,
		float *comboMatrix,
		bool normalize);
};

