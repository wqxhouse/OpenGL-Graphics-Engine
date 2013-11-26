#pragma once
#include <vector>
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Bounds.h"

class Mesh
{
public:
	Mesh(void);
	explicit Mesh(const char *name);
	virtual ~Mesh(void);

	int load(const char *name);

	//struct Edge {
	//	Vector3 v[2];
	//	char reverse;
	//	char flag;
	//};

	//struct Triangle {
	//	Vector3 v[3];			// vertexes
	//	int e[3];			// edges
	//	char reverse[3];	// edge reverse flag
	//	Vector4 plane;			// plane
	//	Vector4 c[3];			// fast point in triangle
	//};

	struct Triangle {
		int v[3];			// vertices
		int cv[3];			// coordinate vertices
		char ce[3];			// convex edges
		Vector4 plane;		// plane
	};


	struct Vertex 
	{
		Vector3 xyz;			// coordinate
		Vector3 normal;		    // normal
		Vector4 tangent;		// tangent
		Vector4 texcoord;		// texture coordinates
	};

	struct Surface {
		char name[128];					// surface name
		Vertex *vertex;
		Vector3 *cvertex;
		std::vector<Triangle*> triangles;
		BBox bbox;				// bounds
		BSphere bsphere;
	};


protected:
	BBox bbox_;					// mesh bounds
	BSphere bsphere_;
};

