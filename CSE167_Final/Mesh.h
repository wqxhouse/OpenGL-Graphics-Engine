#pragma once
#include <vector>
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Bounds.h"
#include "MeshFileObj.h"

class Mesh
{
public:
	struct Triangle 
	{
		int v[3];			// vertices
		int cv[3];			// coordinate vertices
		char ce[3];			// convex edges
		Vector4 plane;		// plane
	};


	struct Vertex 
	{
		Vector3 xyz;
		Vector3 normal;
		Vector3 tangent;
		Vector3 binormal;
		Vector3 texcoord;
	};

	struct Surface 
	{
		char name[128];					// surface name
		Vertex *vertex;					// TODO:need fix for number of vertex

		int num_cvertex;
		Vector3 *cvertex;				// vertex coordinates, stored for fast bounds creation
		std::vector<Triangle*> triangles;
		BBox bbox;				// bounds
		BSphere bsphere;
	};

	Mesh(void);
	explicit Mesh(const Mesh *mesh);
	virtual ~Mesh(void);

	virtual int render(int pplShading = 0, int surface_id = -1);

	// surfaces
	virtual int getNumSurfaces();
	virtual const char *getSurfaceName(int surface_id);
	int getSurface(const char *name);

	// vertices
	virtual int getNumVertex(int surface_id);
	virtual Vertex *getVertex(int surface_id);

	const Vector3 &getMin(int surface_id = -1);
	const Vector3 &getMax(int surface_id = -1);
	const Vector3 &getCenter(int surface_id = -1);
	float getRadius(int surface_id = -1);

	void addSurface(const char *name,Vertex *vertex,int num_vertex);

	void create_mesh_bounds();
	void create_triangle_strips();

	int getNumStrips(int s);
	int *getIndices(int s);

	int getNumTriangles(int s);
	Triangle *getTriangles(int s);

	int getIntersection(const Vector3 &l0, const Vector3 &l1, Vector3 *point, Vector3 *normal, int surface_id = -1);
	void transform_mesh(const Matrix4 &mat);
	void transform_surface(int surface_id);
		
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

	

protected:
	BBox bbox_;					// mesh bounds
	BSphere bsphere_;

	std::vector<Surface*> surfaces_;

	void create_surface_bounds(Surface *s);

};

