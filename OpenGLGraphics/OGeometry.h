#pragma once
#include "object.h"

#include <vector>
#include <string>
#include "Mesh.h"

class OGeometry :
	public Object
{
public:
	explicit OGeometry(Mesh *mesh);
	explicit OGeometry(const char *filename);
	virtual ~OGeometry(void);

	virtual int render(int surface_id = -1, int shadow = -1);

	virtual int getIntersection(
		const Vector3 &l0,
		const Vector3 &l1, 
		Vector3 *point,
		Vector3 *normal, 
		int surface_id = -1) ;

	virtual int getNumSurfaces() ;
	virtual const char* getSurfaceName(int surface_id) ;
	virtual int getSurface(const char *name) ;

	virtual const Vector3 getMin(int surface_id = -1) ;
	virtual const Vector3 getMax(int surface_id = -1) ;

	virtual const Vector3 getCenter(int surface_id = -1) ;
	virtual float getRadius(int surface_id = -1) ;

	Mesh *mesh_;
	std::vector<int> frames_;
};

