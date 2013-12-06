#include "Frustum.h"
#include "Vector3.h"
#include "core.h"
#include <assert.h>

Frustum::Frustum(void)
{
	planes_.resize(6);
}

Frustum::~Frustum(void)
{
}

void Frustum::applyCurrProjMVMat()
{
	Matrix4 &m = Core::projection_.multiplyMat(Core::modelview_);
	float matPOD[16];
	m.getMatrixArray(matPOD);
	extractPlanes(&planes_, matPOD, 1);
}


void Frustum::set(const Matrix4 &m)
{
	float matPOD[16];
	m.getMatrixArray(matPOD);
	//extractPlanes(&planes_, matPOD, 1);   //TODO: bug code <- extractPlane

#define PLANE(n,c0,c1,c2,c3) { \
	planes_[n] = Vector4(c0,c1,c2,c3); \
	planes_[n] = planes_[n].scale(1.0f / Vector3(planes_[n]['x'], planes_[n]['y'], planes_[n]['z']).getLength()); \
	}
	PLANE(0,matPOD[3]-matPOD[0],matPOD[7]-matPOD[4],matPOD[11]-matPOD[8],matPOD[15]-matPOD[12])
		PLANE(1,matPOD[3]+matPOD[0],matPOD[7]+matPOD[4],matPOD[11]+matPOD[8],matPOD[15]+matPOD[12])
		PLANE(2,matPOD[3]-matPOD[1],matPOD[7]-matPOD[5],matPOD[11]-matPOD[9],matPOD[15]-matPOD[13])
		PLANE(3,matPOD[3]+matPOD[1],matPOD[7]+matPOD[5],matPOD[11]+matPOD[9],matPOD[15]+matPOD[13])
		PLANE(4,matPOD[3]-matPOD[2],matPOD[7]-matPOD[6],matPOD[11]-matPOD[10],matPOD[15]-matPOD[14])
		PLANE(5,matPOD[3]+matPOD[2],matPOD[7]+matPOD[6],matPOD[11]+matPOD[10],matPOD[15]+matPOD[14])
#undef PLANE

}

int Frustum::addPortal(const Vector3 *points)
{
	//TODO: investigate, why adding this the following with result screen blinking
	//if(inside(points, 4) == 0)
	//{
	//	//printf("culled from frustum");
	//	return 0;
	//}

	Vector3 cam_pos = Core::camera_.getPosCoord();

	// portal plane
	Vector3 normal = Vector3::Cross(points[1] - points[0], points[2] - points[0]);
	normal.normalize();

	// add portal
	PortalFrustum portal_f;

	float angle = (cam_pos - points[0]).dot(normal);
	if(angle > 0.0f)
	{
		normal = normal.negate();
	}

	portal_f.plane = Vector4(normal['x'], normal['y'], normal['z'], normal.negate().dot(points[0]));

	// clip planes
	int j = (angle > 0.0f) ? 3 : 1;
	for(int i = 0; i < 4; i++) 
	{
		normal = (points[i] - cam_pos).cross(points[j] - cam_pos);
		normal.normalize();
		portal_f.clip_planes[i] = Vector4(normal['x'], normal['y'], normal['z'], normal.negate().dot(cam_pos));
		if(++j == 4)
		{
			j = 0;
		}
	}
	portals_.push_back(portal_f);

	/*static int count = 0;
	for(int i = 0; i < portals_.size(); i++)
	{
	for(int j = 0; j < 4; j++)
	{
	count++;
	printf("plane %d: %.2f, %.2f, %.2f, %.2f\n", 
	count, 
	portals_[i].clip_planes[j]['x'], 
	portals_[i].clip_planes[j]['y'], 
	portals_[i].clip_planes[j]['z'], 
	portals_[i].clip_planes[j]['w']);
	}
	count++;
	printf("plane %d: %.2f, %.2f, %.2f, %.2f\n",
	count, 
	portals_[i].plane['x'], 
	portals_[i].plane['y'], 
	portals_[i].plane['z'], 
	portals_[i].plane['w']);
	}
	printf("\n");
	count = 0;*/

	return 1;
}

void Frustum::removePortal()
{
	assert(portals_.size() > 0 && "Frustum::removePortal(): portals underflow");
	portals_.pop_back();
}

int Frustum::inside(const Vector3 *points, int num) const
{
	//first check if inside portal clip planes
	for(int i = 0; i < portals_.size(); i++)
	{
		if(inside_portal(portals_[i], points, num) == 0)
		{
			return 0;
		}
	}
	//then check frustum bounding
	return inside_frustum_planes(points, num);
}

int Frustum::inside(const Vector3 &min, const Vector3 &max) const
{
	for(int i = 0; i < portals_.size(); i++) 
	{
		if(inside_portal(portals_[i],min,max) == 0) return 0;
	}
	return inside_frustum_planes(min,max);
}

int Frustum::inside_frustum_planes(const Vector3 &min, const Vector3 &max) const
{
	if(inside_plane(planes_[0],min,max) == 0) return 0;
	if(inside_plane(planes_[1],min,max) == 0) return 0;
	if(inside_plane(planes_[2],min,max) == 0) return 0;
	if(inside_plane(planes_[3],min,max) == 0) return 0;
	if(inside_plane(planes_[4],min,max) == 0) return 0;
	if(inside_plane(planes_[5],min,max) == 0) return 0;
	return 1;
}

int Frustum::inside_portal(const PortalFrustum &p,const Vector3 &min,const Vector3 &max) const 
{
	if(inside_plane(p.plane, min, max) == 0) return 0;
	if(inside_plane(p.clip_planes[0], min, max) == 0) return 0;
	if(inside_plane(p.clip_planes[1], min, max) == 0) return 0;
	if(inside_plane(p.clip_planes[2], min, max) == 0) return 0;
	if(inside_plane(p.clip_planes[3], min, max) == 0) return 0;
	return 1;
}

int Frustum::inside_plane(const Vector4 &plane, const Vector3 &min, const Vector3 &max) const
{
	/*float min_x = min['x'] * plane['x'];
	float max_x = min['x'] * plane['x'];
	float min_y = min['y'] * plane['y'];
	float max_y = max['y'] * plane['y'];
	float min_zw = min['z'] * plane['z'] + plane['w'];
	float max_zw = max['z'] * plane['z'] + plane['w'];
	float min_min_xy = min_x + min_y;
	float max_min_xy = max_x + min_y;
	float min_max_xy = min_x + max_y;
	float max_max_xy = max_x + max_y;
	if(min_min_xy + min_zw > 0.0f) return 1;
	if(max_min_xy + min_zw > 0.0f) return 1;
	if(min_max_xy + min_zw > 0.0f) return 1;
	if(max_max_xy + min_zw > 0.0f) return 1;
	if(min_min_xy + max_zw > 0.0f) return 1;
	if(max_min_xy + max_zw > 0.0f) return 1;
	if(min_max_xy + max_zw > 0.0f) return 1;
	if(max_max_xy + max_zw > 0.0f) return 1;
	return 0;*/

	if(plane * Vector4(min['x'],min['y'],min['z'],1) > 0)  return 1;
	if(plane * Vector4(min['x'],min['y'],max['z'],1) > 0)  return 1;
	if(plane * Vector4(min['x'],max['y'],min['z'],1) > 0)  return 1;
	if(plane * Vector4(min['x'],max['y'],max['z'],1) > 0)  return 1;
	if(plane * Vector4(max['x'],min['y'],min['z'],1) > 0)  return 1;
	if(plane * Vector4(max['x'],min['y'],max['z'],1) > 0)  return 1;
	if(plane * Vector4(max['x'],max['y'],min['z'],1) > 0)  return 1;
	if(plane * Vector4(max['x'],max['y'],max['z'],1) > 0)  return 1;
	return 0;
}



int Frustum::inside_frustum_planes(const Vector3 *points, int num_points) const
{
	if(inside_plane(planes_[0],points,num_points) == 0) return 0;
	if(inside_plane(planes_[1],points,num_points) == 0) return 0;
	if(inside_plane(planes_[2],points,num_points) == 0) return 0;
	if(inside_plane(planes_[3],points,num_points) == 0) return 0;
	if(inside_plane(planes_[4],points,num_points) == 0) return 0;
	if(inside_plane(planes_[5],points,num_points) == 0) return 0;
	return 1;
}

int Frustum::inside_frustum_planes(const Vector3 &point, float radius) const
{
	/*if(planes_[0].dotVec3(point) < -radius) return 0;
	if(planes_[1].dotVec3(point) < -radius) return 0;
	if(planes_[2].dotVec3(point) < -radius) return 0;
	if(planes_[3].dotVec3(point) < -radius) return 0;
	if(planes_[4].dotVec3(point) < -radius) return 0;
	if(planes_[5].dotVec3(point) < -radius) return 0;*/

	if(planes_[0].dot(Vector4(point, 1)) < -radius) return 0;
	if(planes_[1].dot(Vector4(point, 1)) < -radius) return 0;
	if(planes_[2].dot(Vector4(point, 1)) < -radius) return 0;
	if(planes_[3].dot(Vector4(point, 1)) < -radius) return 0;
	if(planes_[4].dot(Vector4(point, 1)) < -radius) return 0;
	if(planes_[5].dot(Vector4(point, 1)) < -radius) return 0;

	return 1;
}

int Frustum::inside_plane(const Vector4 &plane, const Vector3 *points, int num) const
{
	for(int i = 0; i < num; i++)
	{
		Vector3 pt_i = points[i];
		Vector4 temp = Vector4(pt_i['x'], pt_i['y'], pt_i['z'], 1);
		if(plane.dot(temp) > 0.0f)
		{
			return 1;
		}
	}
	return 0;
}

int Frustum::inside(const Vector3 &point, float radius) const
{
	for(int i = 0; i < portals_.size(); i++)
	{
		if(inside_portal(portals_[i], point, radius) == 0)
		{
			return 0;
		}
	}
	return inside_frustum_planes(point, radius);
}

int Frustum::inside_portal(const PortalFrustum &p, const Vector3 *points, int num_points) const
{
	if(inside_plane(p.plane,points,num_points) == 0) return 0;
	if(inside_plane(p.clip_planes[0], points,num_points) == 0) return 0;
	if(inside_plane(p.clip_planes[1], points,num_points) == 0) return 0;
	if(inside_plane(p.clip_planes[2], points,num_points) == 0) return 0;
	if(inside_plane(p.clip_planes[3], points,num_points) == 0) return 0;
	return 1;
}

int Frustum::inside_portal(const PortalFrustum &p, const Vector3 &point, float radius) const
{
	if(p.clip_planes[0].dotVec3(point) < -radius) return 0;
	if(p.clip_planes[1].dotVec3(point) < -radius) return 0;
	if(p.clip_planes[2].dotVec3(point) < -radius) return 0;
	if(p.clip_planes[3].dotVec3(point) < -radius) return 0;

	return 1;
}


void Frustum::extractPlanes(
	std::vector<Vector4> *p_planes,
	float *comboMatrix,
	bool normalize)
{
	// Left clipping plane    
	(*p_planes)[0].set(comboMatrix[12] + comboMatrix[0], 'x');
	(*p_planes)[0].set(comboMatrix[13] + comboMatrix[1], 'y');
	(*p_planes)[0].set(comboMatrix[14] + comboMatrix[2], 'z');
	(*p_planes)[0].set(comboMatrix[15] + comboMatrix[3], 'w');
	// Right clipping plane
	(*p_planes)[1].set(comboMatrix[12] - comboMatrix[0], 'x');
	(*p_planes)[1].set(comboMatrix[13] - comboMatrix[1], 'y');
	(*p_planes)[1].set(comboMatrix[14] - comboMatrix[2], 'z');
	(*p_planes)[1].set(comboMatrix[15] - comboMatrix[3], 'w');
	// Top clipping plane
	(*p_planes)[2].set(comboMatrix[12] - comboMatrix[4], 'x');
	(*p_planes)[2].set(comboMatrix[13] - comboMatrix[5], 'y');
	(*p_planes)[2].set(comboMatrix[14] - comboMatrix[6], 'z');
	(*p_planes)[2].set(comboMatrix[15] - comboMatrix[7], 'w');
	// Bottom clipping plane
	(*p_planes)[3].set(comboMatrix[12] + comboMatrix[4], 'x');
	(*p_planes)[3].set(comboMatrix[13] + comboMatrix[5], 'y');
	(*p_planes)[3].set(comboMatrix[14] + comboMatrix[6], 'z');
	(*p_planes)[3].set(comboMatrix[15] + comboMatrix[7], 'w');
	// Near clipping plane
	(*p_planes)[4].set(comboMatrix[12] + comboMatrix[8], 'x');
	(*p_planes)[4].set(comboMatrix[13] + comboMatrix[9], 'y');
	(*p_planes)[4].set(comboMatrix[14] + comboMatrix[10], 'z');
	(*p_planes)[4].set(comboMatrix[15] + comboMatrix[11], 'w');
	// Far clipping plane
	(*p_planes)[5].set(comboMatrix[12] - comboMatrix[8], 'x');
	(*p_planes)[5].set(comboMatrix[13] - comboMatrix[9], 'y');
	(*p_planes)[5].set(comboMatrix[14] - comboMatrix[10], 'z');
	(*p_planes)[5].set(comboMatrix[15] - comboMatrix[11], 'w');
	// Normalize the plane equations, if requested
	if (normalize == true)
	{
		(*p_planes)[0].normalizePlane();
		(*p_planes)[1].normalizePlane();
		(*p_planes)[2].normalizePlane();
		(*p_planes)[3].normalizePlane();
		(*p_planes)[4].normalizePlane();
		(*p_planes)[5].normalizePlane();
	}
}