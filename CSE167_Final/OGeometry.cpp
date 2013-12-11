#include "OGeometry.h"
#include "MeshFileObj.h"
#include <assert.h>

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

	//frustum culling 
	if(frame_ != Core::curr_frame_) 
	{
		if(is_identity_) 
		{
			for(int i = 0; i < getNumSurfaces(); i++) 
			{
				auto pos_min = pos_.getPosCoord() + getMin(i);
				auto pos_max = pos_.getPosCoord() + getMax(i);

				bool isInsideFrustum = Core::frustum_->inside(pos_min, pos_max);
				if(isInsideFrustum)
				{
					frames_[i] = Core::curr_frame_;
				}
				else
				{
					frames_[i] = 0;
				}
			}
		} 
		else 
		{
			for(int i = 0; i < getNumSurfaces(); i++) 
			{
				frames_[i] = Core::frustum_->inside(pos_.getPosCoord() + getCenter(i), getRadius(i)) ? Core::curr_frame_ : 0;

				auto point = pos_.getPosCoord() + getCenter(i);
				auto rad = getRadius(i);

				bool isInsideFrustum = Core::frustum_->inside(point, rad);
				if(isInsideFrustum)
				{
					frames_[i] = Core::curr_frame_;
				}
				else
				{
					frames_[i] = 0;
				}
			}
		}
		frame_ = Core::curr_frame_;
	}

	//printf("frames: ");
	//for(int i = 0; i < frames_.size(); i++)
	//{
	//	printf("%d, ", frames_[i]);
	//}
	//printf(" \n");

	//render
	for(int i = 0; i < getNumSurfaces(); i++) 
	{
		if(frames_[i] != Core::curr_frame_)
		{
			continue;
		}
		if(!materials_[i]->enable())
		{
			continue;
		}
		materials_[i]->bind();
		num_triangles += mesh_->render(true ,i);
	
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
