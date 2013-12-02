#pragma once
#include "Mesh.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glext.h>
#include "MeshFileObj.h"

class MeshVBO
	: public Mesh
{
public:

	explicit MeshVBO(const Mesh *mesh);
	explicit MeshVBO(const MeshFileOBJ &objmesh);
	virtual ~MeshVBO(void);

	virtual int render(int pplShading = 0, int surface_id = -1);

protected:
	std::vector<GLuint> vbo_id_;
};

