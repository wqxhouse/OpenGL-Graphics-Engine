#include "Mesh.h"

Mesh::Mesh(void)
	: bbox_(), 
	  bsphere_(), 
	  surfaces_()
{

}

Mesh::Mesh(const Mesh *mesh)
	: bbox_(), 
	  bsphere_(), 
	  surfaces_()
{
	//TODO: copy a mesh
}

Mesh::~Mesh(void)
{
}

void Mesh::create_surface_bounds(Surface *s)
{
	s->bbox.clear();
	s->bbox.expand(s->cvertex, s->num_cvertex);

	//TOOD: corner cases?
	s->bsphere.clear();
	s->bsphere.expand(s->cvertex, s->num_cvertex);
}

void Mesh::create_mesh_bounds()
{
	for(int i = 0; i < surfaces_.size(); i++)
	{
		Surface *s = surfaces_[i];
		create_surface_bounds(s);

		//mesh bounds computed after surface bounds generated
		bbox_.expand(s->bbox);
		bsphere_.expand(s->bsphere);
	}
}

void Mesh::create_triangle_strips()
{

}

int Mesh::render(int pplShading = 0, int surface_id = -1)
{
	//to be overriden
	printf("Mesh::render() To be overriden..");
	return 0;
}

