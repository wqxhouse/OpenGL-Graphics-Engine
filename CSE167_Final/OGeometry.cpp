#include "OGeometry.h"
#include "MeshFileObj.h"

OGeometry::OGeometry(Mesh *mesh) 
	: Object(OBJ_GEOMETRY), 
	  mesh_(mesh)
{
	materials_.resize(getNumSurfaces());
	frames_.resize(getNumSurfaces());
}

OGeometry::~OGeometry(void)
{
	materials_.clear();
	frames_.clear();
	delete mesh_;
}

OGeometry::OGeometry(const char *filename)
	: Object(OBJ_GEOMETRY)
{
	//TODO: fill meshobj
	MeshFileOBJ objfile;
	objfile.load(filename);
	mesh_ = new Mesh(objfile);

	materials_.resize(getNumSurfaces());
	frames_.resize(getNumSurfaces());
}

/*
 */
int OGeometry::render(int surface_id) 
{
	int num_triangles = 0;
	enable();

	if(frame_ != Core::curr_frame_) 
	{
		if(is_identity_) 
		{
			for(int i = 0; i < getNumSurfaces(); i++) 
			{
				frames_[i] = Core::frustum_->inside(pos_.getPosCoord() + getMin(i), pos_.getPosCoord() + getMax(i)) ? Core::curr_frame_ : 0;
			}
		} 
		else 
		{
			for(int i = 0; i < getNumSurfaces(); i++) 
			{
				frames_[i] = Core::frustum_->inside(pos_.getPosCoord() + getCenter(i), getRadius(i)) ? Core::curr_frame_ : 0;
			}
		}
		frame_ = Core::curr_frame_;
	}

	//render
	for(int i = 0; i < getNumSurfaces(); i++) 
	{
		if(frames_[i] != Core::curr_frame_)
		{
			continue;
		}
		num_triangles += mesh_->render(false,i);
	}
	
	
	disable();
	return num_triangles;
}


/*
 */
int OGeometry::getIntersection(const Vector3 &l0,const Vector3 &l1,Vector3 *point,Vector3 *normal,int surface_id) 
{
	return mesh_->getIntersection(l0, l1, point, normal, surface_id);
}

/*
 */
int OGeometry::getNumSurfaces() 
{
	return mesh_->getNumSurfaces();
}

const char* OGeometry::getSurfaceName( int surface_id )
{
	return mesh_->getSurfaceName(surface_id);
}

int OGeometry::getSurface(const char *name) 
{
	return mesh_->getSurfaceId(name);
}

/*
 */
const Vector3 OGeometry::getMin(int surface_id) 
{
	return mesh_->getMin(surface_id);
}

const Vector3 OGeometry::getMax(int surface_id) 
{
	return mesh_->getMax(surface_id);
}

const Vector3 OGeometry::getCenter(int surface_id) 
{
	return mesh_->getCenter(surface_id);
}

float OGeometry::getRadius(int surface_id) 
{
	return mesh_->getRadius(surface_id);
}
