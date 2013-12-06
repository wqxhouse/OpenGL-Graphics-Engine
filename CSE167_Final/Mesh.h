#pragma once
#include <vector>
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Bounds.h"

class MeshFileOBJ;
class Mesh
{
public:
	struct Triangle 
	{
		Vector3 v[3];			// vertexes
		int e[3];			// edges
		char reverse[3];	// edge reverse flag
		Vector4 plane;			// plane
		Vector4 c[3];			// fast point in triangle

		~Triangle()
		{
			if(v)
			{
				delete[] v;
			}
			if(e)
			{
				delete[] e;
			}
			if(c)
			{
				delete[] c;
			}
		}
	};


	struct Vertex 
	{
		Vector3 xyz;
		Vector3 normal;
		Vector3 tangent;
		Vector3 binormal;
		Vector3 texcoord;
	};

	struct Edge 
	{
		Vector3 v[2];
		bool reverse;
		bool flag;

		~Edge()
		{
			if(v)
			{
				delete[] v;
			}
		}
	};

	struct Surface 
	{
		char name[256];			// surface name
		int num_vertex;	
		Vertex *vertex;					// TODO:need fix for number of vertex
		int num_cvertex;
		Vector3 *cvertex;				// vertex coordinates, stored for fast bounds creation
		int num_triangles;
		Triangle *triangles;
		int num_edges;								// number of edges
		Edge *edges;
		int *indices;
		int num_indices;							// number of indices
		int num_strips;								// number of triangle strips

		BBox bbox;				// bounds
		BSphere bsphere;

		~Surface()
		{
			if(vertex)
			{
				delete[] vertex;
			}
			if(cvertex)
			{
				delete[] cvertex;
			}
			if(triangles)
			{
				delete[] triangles;
			}
			if(edges)
			{
				delete[] edges;
			}
			if(indices)
			{
				delete[] indices;
			}
		}
	};

	Mesh(void);
	explicit Mesh(const MeshFileOBJ &objmesh, bool isWorld = false);
	virtual ~Mesh(void);

	virtual int render(int pplShading = 0, int surface_id = -1);

	virtual void clearMesh();

	// surfaces
	virtual int getNumSurfaces() const;
	virtual const char* getSurfaceName(int surface_id) const;
	int getSurfaceId(const char *name) const;

	int getNumStrips(int s) const;
	int *getIndices(int s) const;

	int getNumEdges(int s) const;
	Edge *getEdges(int s) const;

	int getNumTriangles(int s) const;
	Triangle *getTriangles(int s) const;


	// vertices
	virtual int getNumVertex(int surface_id) const;
	virtual Vertex *getVertex(int surface_id) const;

	const Vector3 getMin(int surface_id = -1);
	const Vector3 getMax(int surface_id = -1);
	const Vector3 getCenter(int surface_id = -1);
	float getRadius(int surface_id = -1);

	void addSurface(const char *name, Vertex *vertex, int num_vertex);
	void addSurface(Mesh *mesh,int surface_id);

	void create_mesh_bounds();
	void create_triangle_strips();
	void create_triangles();
	void create_tangent();

	int getIntersection(const Vector3 &l0, const Vector3 &l1, Vector3 *point, Vector3 *normal, int surface_id = -1);
	void transform_mesh(const Matrix4 &m);
	void transform_surface(const Matrix4 &m, int surface_id);
	int load_mesh_mesh(const char *filename);
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
	explicit Mesh(const Mesh *mesh);

	BBox bbox_;					// mesh bounds
	BSphere bsphere_;

	std::vector<Surface*> surfaces_;

	void create_surface_bounds(Surface *s);
	void init_surface(Surface *surface);

	void load_obj_mesh(const MeshFileOBJ &objmesh);

};

