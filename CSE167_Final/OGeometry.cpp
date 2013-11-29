#include "OGeometry.h"


OGeometry::OGeometry(Mesh *mesh) 
	: Object(OBJ_GEOMETRY)
{

}
OGeometry::OGeometry(const char *filename)
	: Object(OBJ_GEOMETRY)
{

}

OGeometry::~OGeometry(void)
{
	delete mesh_;
}
