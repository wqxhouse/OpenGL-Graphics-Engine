#pragma once
#include "MeshFile.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include "Vector3.h"

class MeshFileOBJ : 
	public MeshFile 
{
public:

	MeshFileOBJ();
	virtual ~MeshFileOBJ();

	// loader
	int load(const char *name);

	// surfaces
	virtual int getNumSurfaces();
	virtual const char *getSurfaceName(int surface);

	// vertices
	virtual int getNumVertex(int surface);
	virtual Mesh::Vertex *getVertex(int surface);

private:

	struct Vertex;

	int skip_spaces(const char *s);
	int read_index(const char *s,int &id);
	int read_vertex(const char *s,Vertex &v);

	void calculate_normals();

	std::vector<Vector3> vertices;
	std::vector<Vector3> texcoords;
	std::vector<Vector3> normals;

	struct Vertex 
	{
		int v;
		int vt;
		int vn;
	};

	struct Surface {
		std::string name;
		std::vector<Vertex> vertex;
	};

	std::vector<Surface> surfaces;

	std::ifstream file;
};