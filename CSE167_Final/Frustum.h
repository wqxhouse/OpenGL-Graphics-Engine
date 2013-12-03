#pragma once
#include <vector>
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"

class Frustum
{
public:
	struct PortalFrustum 
	{
		Vector4 plane;							// portal plane
		Vector4 clip_planes[4];				    // clip planes
		Vector3 points[4];						// bounding points
	};

	Frustum();
	~Frustum();

	void applyCurrProjMVMat();
	void set(const Matrix4 &m);

	int addPortal(const Vector3 *points);
	void removePortal();

	int inside(const Vector3 &min, const Vector3 &max) const;
	int inside(const Vector3 &point, float radius) const;
	int inside(const Vector3 *points, int num) const;

	int inside_portal(const PortalFrustum &p, const Vector3 *points, int num_points) const;
	int inside_portal(const PortalFrustum &p, const Vector3 &point, float radius) const;
	int inside_portal(const PortalFrustum &p, const Vector3 &min, const Vector3 &max) const;
	int inside_frustum_planes(const Vector3 *points, int num_points) const;
	int inside_frustum_planes(const Vector3 &point, float radius) const;
	int inside_frustum_planes(const Vector3 &min , const Vector3 &max) const;
	int inside_plane(const Vector4 &plane, const Vector3 *points, int num) const;
	int inside_plane(const Vector4 &plane, const Vector3 &min, const Vector3 &max) const;
	/*int inside(const Vector3 &light, float light_radius, const Vector3 &center, float radius);
	int inside_all(const Vector3 &min,const Vector3 &max);
	int inside_all(const Vector3 &center,float radius);*/


protected:

	//needs to refactor these constants into codes
	enum 
	{
		PLANE_LEFT = 0, 
		PLANE_RIGHT, 
		PLANE_TOP, 
		PLANE_BOTTOM, 
		PLANE_NEAR, 
		PLANE_FAR
	};

	std::vector<Vector4> planes_;
	std::vector<PortalFrustum> portals_;

	void extractPlanes(
		std::vector<Vector4> *p_planes,
		float *comboMatrix,
		bool normalize);
};

