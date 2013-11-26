#pragma once
#include "Mesh.h"

class MeshFile 
	: public Mesh
{
public:

	virtual ~MeshFile();

	// create new mesh file
	static MeshFile *load(const char *name);

	// surfaces
	virtual int getNumSurfaces() = 0;
	virtual const char *getSurfaceName(int surface) = 0;

	// vertices
	virtual int getNumVertex(int surface) = 0;
	virtual Mesh::Vertex *getVertex(int surface) = 0;
};