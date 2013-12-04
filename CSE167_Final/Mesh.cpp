#include "Mesh.h"
#include <assert.h>
#include "MeshFileObj.h"
#include "BSPTree.h"

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
	clearMesh();

	bbox_ = mesh->bbox_;
	bsphere_ = mesh->bsphere_;

	//note that all num_ * are not being copied, 
	//this is only used as a protected contructor for subclass to fill in num_*
	for(int i = 0; i < mesh->surfaces_.size(); i++) 
	{
		Surface *s = new Surface;
		init_surface(s);

		memcpy(s, mesh->surfaces_[i], sizeof(Surface));

		// name
		strcpy(s->name, mesh->getSurfaceName(i));

		s->vertex = new Vertex[s->num_vertex];
		memcpy(s->vertex,mesh->surfaces_[i]->vertex,sizeof(Vertex) * s->num_vertex);

		s->cvertex = new Vector3[s->num_cvertex];
		memcpy(s->cvertex, mesh->surfaces_[i]->cvertex, sizeof(Vector3) * s->num_cvertex);
		
		if(mesh->surfaces_[i]->edges) 
		{
			s->edges = new Edge[s->num_edges];
			memcpy(s->edges,mesh->surfaces_[i]->edges,sizeof(Edge) * s->num_edges);
		}
		if(mesh->surfaces_[i]->triangles)
		{
			s->triangles = new Triangle[s->num_triangles];
			memcpy(s->triangles,mesh->surfaces_[i]->triangles,sizeof(Triangle) * s->num_triangles);
		}
		if(mesh->surfaces_[i]->indices) 
		{
			s->indices = new int[s->num_indices];
			memcpy(s->indices,mesh->surfaces_[i]->indices,sizeof(int) * s->num_indices);
		}
		
		surfaces_.push_back(s);
	}
}

Mesh::Mesh(const MeshFileOBJ &objmesh, bool isWorld /* = false*/)
{
	clearMesh();
	load_obj_mesh(objmesh);

	create_tangent();
	if(!isWorld)
	{
		create_triangles();
		create_triangle_strips();
	}
	create_mesh_bounds();
}

void Mesh::load_obj_mesh(const MeshFileOBJ &objmesh)
{
	// number of surfaces
	int num_surfaces = objmesh.getNumSurfaces();
	for(int i = 0; i < num_surfaces; i++) 
	{
		Surface *s = new Surface;
		init_surface(s);

		// name
		strcpy(s->name, objmesh.getSurfaceName(i));
		// Vertex
		int num_vertex			= objmesh.getNumVertex(i);
		s->vertex				= new Vertex[num_vertex];
		s->num_vertex			= num_vertex;
		Vertex *vtxArr_sur_i    = objmesh.getVertex(i);
		assert(vtxArr_sur_i && "Mesh::load_obj_mesh, vtxArr_sur_i is nullptr");
		std::copy(vtxArr_sur_i, vtxArr_sur_i + num_vertex, s->vertex);
		delete []vtxArr_sur_i; // free on the fly vertices created in objmesh.getVertex()
		// cvertex
		s->num_cvertex = num_vertex;
		s->cvertex     = new Vector3[s->num_cvertex];
		//unroll s->vertex array to fill in vertex coordinates 
		for(int q = 0; q < s->num_cvertex; q++)
		{
			s->cvertex[q] = s->vertex[q].xyz;
		}
		surfaces_.push_back(s);
	}
}

Mesh::~Mesh(void)
{
}

void Mesh::create_surface_bounds(Surface *s)
{
	s->bbox.clear();
	s->bbox.expand(s->cvertex, s->num_cvertex);

	//TODO: corner cases?
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

//Triangle Structure //////////////////////////////////////////////////////////////////////////
struct csv_Vertex 
{
	Vector3 xyz;
	int id;
};

struct csv_Edge 
{
	int v[2];
	int id;
};

static int csv_vertex_cmp(const void *a,const void *b) {
	csv_Vertex *v0 = (csv_Vertex*)a;
	csv_Vertex *v1 = (csv_Vertex*)b;
	float d;
	d = v0->xyz['x'] - v1->xyz['x'];
	if(d > MATH_EPSILON) return 1;
	if(d < -MATH_EPSILON) return -1;
	d = v0->xyz['y'] - v1->xyz['y'];
	if(d > MATH_EPSILON) return 1;
	if(d < -MATH_EPSILON) return -1;
	d = v0->xyz['z'] - v1->xyz['z'];
	if(d > MATH_EPSILON) return 1;
	if(d < -MATH_EPSILON) return -1;
	return 0;
}

static int csv_edge_cmp(const void *a,const void *b) {
	csv_Edge *e0 = (csv_Edge*)a;
	csv_Edge *e1 = (csv_Edge*)b;
	int v[2][2];
	if(e0->v[0] < e0->v[1]) { v[0][0] = e0->v[0]; v[0][1] = e0->v[1]; }
	else { v[0][0] = e0->v[1]; v[0][1] = e0->v[0]; }
	if(e1->v[0] < e1->v[1]) { v[1][0] = e1->v[0]; v[1][1] = e1->v[1]; }
	else { v[1][0] = e1->v[1]; v[1][1] = e1->v[0]; }
	if(v[0][0] > v[1][0]) return 1;
	if(v[0][0] < v[1][0]) return -1;
	if(v[0][1] > v[1][1]) return 1;
	if(v[0][1] < v[1][1]) return -1;
	return 0;
}

void Mesh::create_triangles() 
{
	for(int i = 0; i < surfaces_.size(); i++) 
	{
		Surface *s = surfaces_[i];
		s->num_edges = s->num_vertex;
		s->num_triangles = s->num_vertex / 3;

		// cerate vertex
		csv_Vertex *v = new csv_Vertex[s->num_vertex];
		for(int j = 0; j < s->num_vertex; j++) 
		{
			v[j].xyz = s->vertex[j].xyz;
			v[j].id = j;
		}
		qsort(v,s->num_vertex,sizeof(csv_Vertex),csv_vertex_cmp);
		int num_optimized_vertex = 0;
		int *vbuf = new int[s->num_vertex];
		for(int j = 0; j < s->num_vertex; j++) 
		{
			if(j == 0 || csv_vertex_cmp(&v[num_optimized_vertex - 1],&v[j])) v[num_optimized_vertex++] = v[j];
			vbuf[v[j].id] = num_optimized_vertex - 1;
		}
		// create edges
		csv_Edge *e = new csv_Edge[s->num_edges];
		for(int j = 0; j < s->num_edges; j += 3)
		{
			e[j + 0].v[0] = vbuf[j + 0];
			e[j + 0].v[1] = vbuf[j + 1];
			e[j + 1].v[0] = vbuf[j + 1];
			e[j + 1].v[1] = vbuf[j + 2];
			e[j + 2].v[0] = vbuf[j + 2];
			e[j + 2].v[1] = vbuf[j + 0];
			e[j + 0].id = j + 0;
			e[j + 1].id = j + 1;
			e[j + 2].id = j + 2;
		}
		qsort(e,s->num_edges,sizeof(csv_Edge),csv_edge_cmp);
		int num_optimized_edges = 0;
		int *ebuf = new int[s->num_edges];
		int *rbuf = new int[s->num_edges];
		for(int j = 0; j < s->num_edges; j++) {
			if(j == 0 || csv_edge_cmp(&e[num_optimized_edges - 1],&e[j])) e[num_optimized_edges++] = e[j];
			ebuf[e[j].id] = num_optimized_edges - 1;
			rbuf[e[j].id] = e[num_optimized_edges - 1].v[0] != e[j].v[0];
		}
		// final
		s->num_edges = num_optimized_edges;
		s->edges = new Edge[s->num_edges];
		for(int j = 0; j < s->num_edges; j++) {
			s->edges[j].v[0] = v[e[j].v[0]].xyz;
			s->edges[j].v[1] = v[e[j].v[1]].xyz;
			s->edges[j].reverse = 0;
			s->edges[j].flag = 0;
		}

		// triangles
		s->triangles = new Triangle[s->num_triangles];
		for(int j = 0, k = 0; j < s->num_triangles; j++, k += 3) 
		{
			Triangle *t = &s->triangles[j];
			t->v[0] = v[vbuf[k + 0]].xyz;
			t->v[1] = v[vbuf[k + 1]].xyz;
			t->v[2] = v[vbuf[k + 2]].xyz;
			t->e[0] = ebuf[k + 0];
			t->e[1] = ebuf[k + 1];
			t->e[2] = ebuf[k + 2];
			t->reverse[0] = rbuf[k + 0];
			t->reverse[1] = rbuf[k + 1];
			t->reverse[2] = rbuf[k + 2];
			Vector3 normal;
			normal = Vector3::Cross(t->v[1] - t->v[0],t->v[2] - t->v[0]);
			normal.normalize();
			t->plane = Vector4(normal, t->v[0].negate().dot(normal));
			normal = Vector3::Cross(t->plane.getVector3(),t->v[0] - t->v[2]);	// fast point in traingle
			normal.normalize();
			t->c[0] = Vector4(normal, t->v[0].negate().dot(normal));
			normal = Vector3::Cross(t->plane.getVector3(), t->v[1] - t->v[0]);
			normal.normalize();
			t->c[1] = Vector4(normal, t->v[1].negate().dot(normal));
			normal = Vector3::Cross(t->plane.getVector3(), t->v[2] - t->v[1]);
			normal.normalize();
			t->c[2] = Vector4(normal, t->v[2].negate().dot(normal));
		}

		delete[] rbuf;
		delete[] ebuf;
		delete[] e;
		delete[] v;
	}

}


//////////////////////////////////////////////////////////////////////////
// Triangle Strips
//////////////////////////////////////////////////////////////////////////

struct cts_Vertex {
	Mesh::Vertex v;
	int face;
	int num;
	int id;
};

struct cts_Edge {
	int v[2];
	int face;
	int num;
};

struct cts_Triangle {
	int v[3];
	int env[3];
	int flag;
};

static int cts_vertex_cmp(const void *a,const void *b) {
	cts_Vertex *ctsv0 = (cts_Vertex*)a;
	cts_Vertex *ctsv1 = (cts_Vertex*)b;
	Mesh::Vertex *v0 = &ctsv0->v;
	Mesh::Vertex *v1 = &ctsv1->v;
	float d;
#define CMP(a,b,e) { \
	d = (a)['x'] - (b)['x']; \
	if(d > e) return 1; \
	if(d < -e) return -1; \
	d = (a)['y'] - (b)['y']; \
	if(d > e) return 1; \
	if(d < -e) return -1; \
	d = (a)['z'] - (b)['z']; \
	if(d > e) return 1; \
	if(d < -e) return -1; \
	}
	CMP(v0->xyz,v1->xyz,MATH_EPSILON)
		CMP(v0->normal,v1->normal,MATH_EPSILON)
		//CMP(v0->tangent,v1->tangent,0.1)
		//CMP(v0->binormal,v1->binormal,0.1)
#undef CMP
		d = v0->texcoord['x'] - v1->texcoord['x'];
	if(d > MATH_EPSILON) return 1;
	if(d < -MATH_EPSILON) return -1;
	d = v0->texcoord['y'] - v1->texcoord['y'];
	if(d > MATH_EPSILON) return 1;
	if(d < -MATH_EPSILON) return -1;
	return 0;
}

static int cts_edge_cmp(const void *a,const void *b) {
	cts_Edge *e0 = (cts_Edge*)a;
	cts_Edge *e1 = (cts_Edge*)b;
	int v[2][2];
	if(e0->v[0] < e0->v[1]) { v[0][0] = e0->v[0]; v[0][1] = e0->v[1]; }
	else { v[0][0] = e0->v[1]; v[0][1] = e0->v[0]; }
	if(e1->v[0] < e1->v[1]) { v[1][0] = e1->v[0]; v[1][1] = e1->v[1]; }
	else { v[1][0] = e1->v[1]; v[1][1] = e1->v[0]; }
	if(v[0][0] > v[1][0]) return 1;
	if(v[0][0] < v[1][0]) return -1;
	if(v[0][1] > v[1][1]) return 1;
	if(v[0][1] < v[1][1]) return -1;
	return 0;
}

void Mesh::create_triangle_strips()
{
	for(int i = 0; i < surfaces_.size(); i++) 
	{
		Surface *s = surfaces_[i];
		s->num_indices = 0;	// already is zero
		s->num_strips = 0;
		s->indices = new int[s->num_vertex * 4 / 3];
		cts_Vertex *v = new cts_Vertex[s->num_vertex];
		cts_Edge *e = new cts_Edge[s->num_vertex];
		cts_Triangle *t = new cts_Triangle[s->num_vertex / 3];
		// create vertex
		for(int j = 0; j < s->num_vertex; j++) 
		{
			v[j].v = s->vertex[j];
			v[j].face = j / 3;
			v[j].num = j % 3;
			v[j].id = j;
		}
		qsort(v,s->num_vertex,sizeof(cts_Vertex),cts_vertex_cmp);
		int num_optimized_vertex = 0;
		int *vbuf = new int[s->num_vertex];
		for(int j = 0; j < s->num_vertex; j++) 
		{
			if(j == 0 || cts_vertex_cmp(&v[num_optimized_vertex - 1],&v[j])) v[num_optimized_vertex++] = v[j];
			t[v[j].face].v[v[j].num] = num_optimized_vertex - 1;
			vbuf[v[j].id] = num_optimized_vertex - 1;
		}
		// create edges
		for(int j = 0; j < s->num_vertex; j += 3) 
		{
			e[j + 0].v[0] = vbuf[j + 1];
			e[j + 0].v[1] = vbuf[j + 2];
			e[j + 1].v[0] = vbuf[j + 0];
			e[j + 1].v[1] = vbuf[j + 2];
			e[j + 2].v[0] = vbuf[j + 0];
			e[j + 2].v[1] = vbuf[j + 1];
			e[j + 0].face = e[j + 1].face = e[j + 2].face = j / 3;
			e[j + 0].num = 0;
			e[j + 1].num = 1;
			e[j + 2].num = 2;
		}

		delete vbuf;
		qsort(e,s->num_vertex,sizeof(cts_Edge),cts_edge_cmp);

		// create triangles
		for(int j = 0; j < s->num_vertex / 3; j++) 
		{
			t[j].env[0] = t[j].env[1] = t[j].env[2] = -1;
			t[j].flag = 0;
		}

		// find triangle environment
		for(int j = 0, k = 0; j < s->num_vertex; j++)
		{
			if(j == 0 || cts_edge_cmp(&e[k],&e[j])) k = j;
			if(j != k) {
				t[e[j].face].env[e[j].num] = e[k].face;
				t[e[k].face].env[e[k].num] = e[j].face;
			}
		}
		delete e;

		// create triangle strips
		for(int j = 0; j < s->num_vertex / 3; j++) 
		{
			if(t[j].v[0] == t[j].v[1] || t[j].v[1] == t[j].v[2] || t[j].v[2] == t[j].v[0]) t[j].flag = 1;
		}
		int *indices = s->indices;
		for(int j = 0; j < s->num_vertex / 3; j++) 
		{
			if(!t[j].flag) 
			{
				s->num_strips++;
				int env;
				for(env = 0; env < 3; env++) if(t[j].env[env] != -1 && t[t[j].env[env]].flag == 0) break;
				int *strip_length = indices++;
				*strip_length = 3;
				*indices++ = t[j].v[(0 + env) % 3];
				*indices++ = t[j].v[(1 + env) % 3];
				*indices++ = t[j].v[(2 + env) % 3];
				t[j].flag = 1;
				int k = j;
				while(1) 
				{
					for(env = 0; env < 3; env++) 
					{
						if(t[k].env[env] != -1 && t[t[k].env[env]].flag == 0) {
							int *v = t[t[k].env[env]].v;
							if(*(indices - 3) != v[0] && ((*(indices - 2) == v[2] && *(indices - 1) == v[1]) || (*(indices - 2) == v[1] && *(indices - 1) == v[2]))) 
							{
								*indices++ = v[0];
								break;
							}
							if(*(indices - 3) != v[2] && ((*(indices - 2) == v[1] && *(indices - 1) == v[0]) || (*(indices - 2) == v[0] && *(indices - 1) == v[1]))) 
							{
								*indices++ = v[2];
								break;
							}
							if(*(indices - 3) != v[1] && ((*(indices - 2) == v[0] && *(indices - 1) == v[2]) || (*(indices - 2) == v[2] && *(indices - 1) == v[0]))) 
							{
								*indices++ = v[1];
								break;
							}
						}
					}

					if(env == 3) break;
					Vector3 normal;
					normal = Vector3::Cross(v[*(indices - 3)].v.xyz 
						                  - v[*(indices - 2)].v.xyz, 
										    v[*(indices - 4)].v.xyz 
										  - v[*(indices - 2)].v.xyz);

					normal = Vector3::Cross(normal, v[*(indices - 3)].v.xyz - v[*(indices - 2)].v.xyz);

					Vector4 plane(normal['x'], normal['y'], normal['z'], -(v[*(indices - 2)].v.xyz.dot(normal)));
					if((plane.dotVec3(v[*(indices - 4)].v.xyz)) * (plane.dotVec3(v[*(indices - 1)].v.xyz)) > 0.0) 
					{
						indices--;
						break;
					}
					k = t[k].env[env];
					t[k].flag = 1;
					(*strip_length)++;
				}
				s->num_indices += (*strip_length) + 1;
			}
		}
		delete t;
		delete []s->vertex;
		s->num_vertex = num_optimized_vertex;
		s->vertex = new Vertex[s->num_vertex];
		for(int j = 0; j < s->num_vertex; j++)
		{
			s->vertex[j] = v[j].v;
		}
		delete []v;
		indices = new int[s->num_indices];
		for(int j = 0; j < s->num_indices; j++)
		{
			indices[j] = s->indices[j];
		}
		delete []s->indices;
		s->indices = indices;
	}
}

void Mesh::create_tangent()
{
	for(int i = 0; i < surfaces_.size(); i++) 
	{
		Surface *s = surfaces_[i];
		for(int j = 0; j < s->num_vertex; j += 3) 
		{
			Vertex *v0 = &s->vertex[j + 0];
			Vertex *v1 = &s->vertex[j + 1];
			Vertex *v2 = &s->vertex[j + 2];

			Vector3 normal, tangent, binormal;

			Vector3 e0 = Vector3(0,v1->texcoord['x'] - v0->texcoord['x'],v1->texcoord['y'] - v0->texcoord['y']);
			Vector3 e1 = Vector3(0,v2->texcoord['x'] - v0->texcoord['x'],v2->texcoord['y'] - v0->texcoord['y']);
			for(int k = 0; k < 3; k++) 
			{
				e0.set(v1->xyz[k + 'x'] - v0->xyz[k + 'x'], 'x');
				e1.set(v2->xyz[k + 'x'] - v0->xyz[k + 'x'], 'y');
				Vector3 v = Vector3::Cross(e0, e1);

				if(std::fabs(v['x']) > MATH_EPSILON) 
				{
					tangent.set (-v['y'] / v['x'], k + 'x');
					binormal.set(-v['z'] / v['x'], k + 'x');
				} 
				else 
				{
					tangent.set(0, k + 'x');
					binormal.set(0, k + 'x');
				}
			}

			tangent.normalize();
			binormal.normalize();
			normal = Vector3::Cross(tangent, binormal);
			normal.normalize();

			v0->binormal = Vector3::Cross(v0->normal, tangent);
			v0->binormal.normalize();
			v0->tangent = Vector3::Cross(v0->binormal, v0->normal);

			if(normal.dot(v0->normal) < 0) 
			{
				v0->binormal = v0->binormal.negate();
			}

			v1->binormal = Vector3::Cross(v1->normal, tangent);
			v1->binormal.normalize();
			v1->tangent = Vector3::Cross(v1->binormal, v1->normal);

			if(normal.dot(v1->normal) < 0)
			{
				v1->binormal = v1->binormal.negate();
			}

			v2->binormal = Vector3::Cross(v2->normal, tangent);
			v2->binormal.normalize();
			v2->tangent = Vector3::Cross(v2->binormal, v2->normal);

			if(normal.dot(v2->normal) < 0)
			{
				v2->binormal = v2->binormal.negate();
			}
		}
	}
}

int Mesh::render(int pplShading, int surface_id)
{
	//to be overriden
	printf("Mesh::render() To be overriden..");
	return 0;
}

void Mesh::clearMesh()
{
	bbox_.clear();
	bsphere_.clear();
	surfaces_.clear();
}

void Mesh::init_surface(Surface *surface)
{
	memset(surface->name, 0, 256);
	surface->vertex = nullptr;
	surface->num_vertex = 0;
	surface->cvertex = nullptr;
	surface->num_cvertex = 0;
	surface->triangles = nullptr;
	surface->num_triangles = 0;
	surface->edges = nullptr;
	surface->num_edges = 0;
	surface->num_strips = 0;
	surface->indices = nullptr;
	surface->num_indices = 0;

	surface->bbox.clear();
	surface->bsphere.clear();
}

//////////////////////////////////////////////////////////////////////////
// Add surface ///
//////////////////////////////////////////////////////////////////////////

void Mesh::addSurface(const char *name, Vertex *vertex, int num_vertex)
{
	Surface *s = new Surface;
	init_surface(s);
	assert(name != nullptr && "Mesh::addSurface, null name");

	//TODO: check whether std::string can be directly converted with cstring
	strcpy(s->name, name);
	
	//copy vertex
	s->num_vertex = num_vertex;
	s->vertex = new Vertex[s->num_vertex];
	memcpy(s->vertex, vertex, sizeof(Vertex) * s->num_vertex);

	//copy cvertex
	s->num_cvertex = num_vertex;
	s->cvertex     = new Vector3[s->num_cvertex];
	//generate from vertex unrolling
	for(int q = 0; q < s->num_cvertex; q++)
	{
		s->cvertex[q] = s->vertex[q].xyz;
	}

	/*if(num_surfaces == NUM_SURFACES) 
	{
	fprintf(stderr,"Mesh::addSurface(): many surfaces\n");
	num_surfaces--;
	}*/
	surfaces_.push_back(s);

	create_mesh_bounds();
}

void Mesh::addSurface(Mesh *mesh, int surface_id)
{
	Surface *s = new Surface;
	init_surface(s);
	memcpy(s, mesh->surfaces_[surface_id], sizeof(Surface));

	//needs to allocate new dyn memory to perform deep copy
	s->vertex = new Vertex[s->num_vertex];
	memcpy(s->vertex,mesh->surfaces_[surface_id]->vertex,sizeof(Vertex) * s->num_vertex);

	s->cvertex = new Vector3[s->num_cvertex];
	memcpy(s->cvertex, mesh->surfaces_[surface_id]->cvertex, sizeof(Vector3) * s->num_cvertex);
	
	if(mesh->surfaces_[surface_id]->edges) 
	{
		s->edges = new Edge[s->num_edges];
		memcpy(s->edges,mesh->surfaces_[surface_id]->edges,sizeof(Edge) * s->num_edges);
	}
	if(mesh->surfaces_[surface_id]->triangles) 
	{
		s->triangles = new Triangle[s->num_triangles];
		memcpy(s->triangles,mesh->surfaces_[surface_id]->triangles,sizeof(Triangle) * s->num_triangles);
	}
	if(mesh->surfaces_[surface_id]->indices) 
	{
		s->indices = new int[s->num_indices];
		memcpy(s->indices,mesh->surfaces_[surface_id]->indices,sizeof(int) * s->num_indices);
	}

	surfaces_.push_back(s);
	create_mesh_bounds();
}

const Vector3 Mesh::getMin( int surface_id /*= -1*/ )
{
	if(surface_id < 0)
	{
		return bbox_.getMin();
	}
	else
	{
		return surfaces_[surface_id]->bbox.getMin();
	}
}

const Vector3 Mesh::getMax( int surface_id /*= -1*/ )
{
	if(surface_id < 0)
	{
		return bbox_.getMax();
	}
	else
	{
		return surfaces_[surface_id]->bbox.getMax();
	}
}

const Vector3 Mesh::getCenter( int surface_id /*= -1*/ )
{
	if(surface_id < 0)
	{
		return bsphere_.getCenter().getVector3();
	}
	else
	{
		return surfaces_[surface_id]->bsphere.getCenter().getVector3();
	}
}

float Mesh::getRadius(int surface_id)
{
	if(surface_id < 0)
	{
		return bsphere_.getRadius();
	}
	else
	{
		return surfaces_[surface_id]->bsphere.getRadius();
	}
}

// surfaces
int Mesh::getNumSurfaces() const
{
	return surfaces_.size();
}

const char* Mesh::getSurfaceName( int surface_id ) const
{
	return surfaces_[surface_id]->name;	
}

int Mesh::getSurfaceId(const char *name) const
{
	for(int i = 0; i < surfaces_.size(); i++) 
	{
		if(!strcmp(name, surfaces_[i]->name))
		{
			return i;
		}
	}
	assert(false && "Mesh::getSurfaceId(): cannot find surface with this id");
	return -1;
}

// vertices
int Mesh::getNumVertex(int surface_id) const
{
	return surfaces_[surface_id]->num_vertex;
}

Mesh::Vertex* Mesh::getVertex(int surface_id) const
{
	return surfaces_[surface_id]->vertex;
}

int Mesh::getNumStrips(int surface_id) const
{
	return surfaces_[surface_id]->num_strips;
}

int *Mesh::getIndices(int surface_id) const
{
	return surfaces_[surface_id]->indices;
}

int Mesh::getNumEdges(int surface_id) const
{
	return surfaces_[surface_id]->num_edges;
}

Mesh::Edge *Mesh::getEdges(int surface_id) const
{
	return surfaces_[surface_id]->edges;
}

int Mesh::getNumTriangles(int surface_id) const
{
	return surfaces_[surface_id]->num_triangles;
}

Mesh::Triangle *Mesh::getTriangles(int surface_id) const
{
	return surfaces_[surface_id]->triangles;
}

int Mesh::getIntersection(const Vector3 &l0, 
	const Vector3 &l1, Vector3 *point, Vector3 *normal, int surface_id)
{
	Vector3 &bs_center = bsphere_.getCenter().getVector3();
	float bs_rad       = bsphere_.getRadius();

	float nearest = 2.0;
	if(surface_id < 0) 
	{
		Vector3 dir = l1 - l0;
		float dotResult = (dir.dot(bs_center) - dir.dot(l0)) / (dir.dot(dir));
		if(dotResult < 0.0 && (l0 - bs_center).getLength() > bs_rad) return 0;
		else if(dotResult > 1.0 && (l1 - bs_center).getLength() > bs_rad) return 0;
		else if((l0 + dir.scale(dotResult) - bs_center).getLength() > bs_rad) return 0;

		for(int i = 0; i < surfaces_.size(); i++) 
		{
			Surface *s = surfaces_[i];
			Vector3 &s_ctr_pos = s->bsphere.getCenter().getVector3();
			float    s_rad     = s->bsphere.getRadius();
			float dotResult = (dir.dot(s_ctr_pos) - dir.dot(l0)) / (dir.dot(dir));

			if(dotResult < 0.0 && (l0 - s_ctr_pos).getLength() > s_rad) continue;
			else if(dotResult > 1.0 && (l1 - s_ctr_pos).getLength() > s_rad) continue;
			else if((l0 + dir.scale(dotResult) - s_ctr_pos).getLength() > s_rad) continue;

			for(int j = 0; j < s->num_triangles; j++) 
			{
				Triangle *t = &s->triangles[j];
				float dotResult = -(t->plane * Vector4(l0['x'], l0['y'], l0['z'], 1)) 
					             / ( Vector3(t->plane['x'], t->plane['y'], t->plane['z']).dot(dir) );

				if(dotResult < 0.0 || dotResult > 1.0) continue;
				Vector3 p = l0 + dir.scale(dotResult);

				if(nearest > dotResult && t->c[0].dotVec3(p) > 0.0 && t->c[1].dotVec3(p) > 0.0 && t->c[2].dotVec3(p) > 0.0) 
				{
					nearest = dotResult;
					*point = p;   //copy
					*normal = Vector3(t->plane['x'], t->plane['y'], t->plane['z']);
				}
			}
		}
		return nearest < 2.0 ? 1 : 0;
	} 
	else 
	{
		Surface *s = surfaces_[surface_id];
		Vector3 &s_ctr_pos = s->bsphere.getCenter().getVector3();
		float    s_rad     = s->bsphere.getRadius();
		Vector3 dir = l1 - l0;

		float dotResult = (dir.dot(bs_center) - dir.dot(l0)) / (dir.dot(dir));
		if(dotResult < 0.0 && (l0 - s_ctr_pos).getLength() > s_rad) return 0;
		else if(dotResult > 1.0 && (l1 - s_ctr_pos).getLength() > s_rad) return 0;
		else if((l0 + dir.scale(dotResult) - s_ctr_pos).getLength() > s_rad) return 0;

		for(int i = 0; i < s->num_triangles; i++) 
		{
			Triangle *t = &s->triangles[i];
			float dot = -(t->plane * Vector4(l0['x'], l0['y'], l0['z'], 1)) 
				/ (Vector3(t->plane['x'], t->plane['y'], t->plane['z']).dot(dir));

			if(dot < 0.0 || dot > 1.0) continue;

			Vector3 p = l0 + dir.scale(dotResult);

			if(nearest > dot && t->c[0].dotVec3(p) > 0.0 && t->c[1].dotVec3(p) > 0.0 && t->c[2].dotVec3(p) > 0.0) 
			{
				nearest = dot;
				*point = p;
				*normal = Vector3(t->plane['x'], t->plane['y'], t->plane['z']);
			}
		}
		return nearest < 2.0 ? 1 : 0;
	}
}

void Mesh::transform_mesh(const Matrix4 &m)
{
	Matrix4 r = m.getRotationPart();
	for(int i = 0; i < surfaces_.size(); i++) 
	{
		Surface *s = surfaces_[i];
		for(int j = 0; j < s->num_vertex; j++) 
		{
			Vertex *v = &s->vertex[j];
			v->xyz = m.multiplyVec3(v->xyz);
			v->normal = r.multiplyVec3(v->normal);
			v->normal.normalize();

			v->tangent = r.multiplyVec3(v->tangent);
			v->tangent.normalize();
			v->binormal = r.multiplyVec3(v->binormal);
			v->binormal.normalize();
		}
		for(int j = 0; j < s->num_edges; j++) 
		{
			Edge *e = &s->edges[j];
			e->v[0] = m.multiplyVec3(e->v[0]);
			e->v[1] = m.multiplyVec3(e->v[1]);
		}
		for(int j = 0; j < s->num_triangles; j++) 
		{
			Triangle *t = &s->triangles[j];
			t->v[0] = m.multiplyVec3(t->v[0]);
			t->v[1] = m.multiplyVec3(t->v[1]);
			t->v[2] = m.multiplyVec3(t->v[2]);

			Vector3 normal;
			normal = Vector3::Cross(t->v[1] - t->v[0], t->v[2] - t->v[0]);
			normal.normalize();
			t->plane = Vector4(normal['x'], normal['y'], normal['z'], t->v[0].negate().dot(normal));

			normal = Vector3::Cross(t->plane.getVector3(), t->v[0] - t->v[2]);	
			normal.normalize();
			t->c[0] = Vector4(normal['x'], normal['y'], normal['z'], t->v[0].negate().dot(normal));

			normal = Vector3::Cross(t->plane.getVector3(), t->v[1] - t->v[0]);
			normal.normalize();
			t->c[1] = Vector4(normal['x'], normal['y'], normal['z'], t->v[1].negate().dot(normal));

			normal = Vector3::Cross(t->plane.getVector3(), t->v[2] - t->v[1]);
			normal.normalize();
			t->c[2] = Vector4(normal['x'], normal['y'], normal['z'], t->v[2].negate().dot(normal));
		}
	}
	create_mesh_bounds();
}

void Mesh::transform_surface(const Matrix4 &m, int surface_id)
{
	Matrix4 r = m.getRotationPart();
	Surface *s = surfaces_[surface_id];
	for(int j = 0; j < s->num_vertex; j++) 
	{
		Vertex *v = &s->vertex[j];
		v->xyz = m.multiplyVec3(v->xyz);
		v->normal = r.multiplyVec3(v->normal);
		v->normal.normalize();

		v->tangent = r.multiplyVec3(v->tangent);
		v->tangent.normalize();
		v->binormal = r.multiplyVec3(v->binormal);
		v->binormal.normalize();
	}
	for(int j = 0; j < s->num_edges; j++) 
	{
		Edge *e = &s->edges[j];
		e->v[0] = m.multiplyVec3(e->v[0]);
		e->v[1] = m.multiplyVec3(e->v[1]);
	}
	for(int j = 0; j < s->num_triangles; j++) 
	{
		Triangle *t = &s->triangles[j];
		t->v[0] = m.multiplyVec3(t->v[0]);
		t->v[1] = m.multiplyVec3(t->v[1]);
		t->v[2] = m.multiplyVec3(t->v[2]);

		Vector3 normal;
		normal = Vector3::Cross(t->v[1] - t->v[0], t->v[2] - t->v[0]);
		normal.normalize();
		t->plane = Vector4(normal['x'], normal['y'], normal['z'], t->v[0].negate().dot(normal));

		normal = Vector3::Cross(t->plane.getVector3(), t->v[0] - t->v[2]);	
		normal.normalize();
		t->c[0] = Vector4(normal['x'], normal['y'], normal['z'], t->v[0].negate().dot(normal));

		normal = Vector3::Cross(t->plane.getVector3(), t->v[1] - t->v[0]);
		normal.normalize();
		t->c[1] = Vector4(normal['x'], normal['y'], normal['z'], t->v[1].negate().dot(normal));

		normal = Vector3::Cross(t->plane.getVector3(), t->v[2] - t->v[1]);
		normal.normalize();
		t->c[2] = Vector4(normal['x'], normal['y'], normal['z'], t->v[2].negate().dot(normal));
	}
	create_mesh_bounds();
}