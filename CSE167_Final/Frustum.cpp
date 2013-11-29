#include "Frustum.h"
#include "Vector3.h"
#include "core.h"

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

void Frustum::addPortal(const Vector3 &point,const Vector3 *points)
{

}

void Frustum::removePortal()
{

}

int Frustum::inside(const Vector3 *points, int num)
{

}

int Frustum::inside(const Vector3 &point, float radius)
{

}

int Frustum::inside_portal(const Portal &p, const Vector3 &point, float radius)
{
	if(portal.planes[0].dot(point) < -radius) return 0;
	if(portal.planes[1].dot(point) < -radius) return 0;
	if(portal.planes[2].dot(point) < -radius) return 0;
	if(portal.planes[3].dot(point) < -radius) return 0;
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