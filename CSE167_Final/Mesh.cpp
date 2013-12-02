#include "Mesh.h"
#include <assert.h>
#include "MeshFileObj.h"

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

	for(int i = 0; i < mesh->surfaces_.size(); i++) 
	{
		Surface *s = new Surface;
		init_surface(s);

		s->vertex = new Vertex[s->num_vertex];
		memcpy(s->vertex,mesh->surfaces_[i]->vertex,sizeof(Vertex) * s->num_vertex);

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

Mesh::Mesh(const MeshFileOBJ &objmesh)
{
	clearMesh();
	load_obj_mesh(objmesh);

	create_tangent();
	create_triangle_strips();
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
		s->name = objmesh.getSurfaceName(i);

		// vertexes
		int num_vertex = objmesh.getNumVertex(i);

		/*load_mesh_Vertex *vertex = new load_mesh_Vertex[num_vertex];
		fread(vertex,sizeof(load_mesh_Vertex),num_vertex,file);*/
		s->vertex = new Vertex[num_vertex];
		Vertex *vtxArr_sur_i = objmesh.getVertex(i);
		std::copy(vtxArr_sur_i, vtxArr_sur_i + num_vertex, s->vertex);
		delete vtxArr_sur_i;

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
	d = (a)[0] - (b)[0]; \
	if(d > e) return 1; \
	if(d < -e) return -1; \
	d = (a)[1] - (b)[1]; \
	if(d > e) return 1; \
	if(d < -e) return -1; \
	d = (a)[2] - (b)[2]; \
	if(d > e) return 1; \
	if(d < -e) return -1; \
	}
	CMP(v0->xyz,v1->xyz,MATH_EPSILON)
		CMP(v0->normal,v1->normal,MATH_EPSILON)
		//CMP(v0->tangent,v1->tangent,0.1)
		//CMP(v0->binormal,v1->binormal,0.1)
#undef CMP
		d = v0->texcoord[0] - v1->texcoord[0];
	if(d > MATH_EPSILON) return 1;
	if(d < -MATH_EPSILON) return -1;
	d = v0->texcoord[1] - v1->texcoord[1];
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
		// create vertexes
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
		delete s->vertex;
		s->num_vertex = num_optimized_vertex;
		s->vertex = new Vertex[s->num_vertex];
		for(int j = 0; j < s->num_vertex; j++)
		{
			s->vertex[j] = v[j].v;
		}
		delete v;
		indices = new int[s->num_indices];
		for(int j = 0; j < s->num_indices; j++)
		{
			indices[j] = s->indices[j];
		}
		delete s->indices;
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
				e0.set(v1->xyz[k] - v0->xyz[k], 'x');
				e1.set(v2->xyz[k] - v0->xyz[k], 'y');
				Vector3 v = Vector3::Cross(e0, e1);

				if(std::fabs(v[0]) > MATH_EPSILON) 
				{
					tangent.set (-v[1] / v[0], k + 'x');
					binormal.set(-v[2] / v[0], k + 'x');
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
	surface->name.clear();
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
	s->name = name;
	
	s->num_vertex = num_vertex;
	s->vertex = new Vertex[s->num_vertex];

	memcpy(s->vertex,vertex, sizeof(Vertex) * s->num_vertex);

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

	s->vertex = new Vertex[s->num_vertex];
	memcpy(s->vertex,mesh->surfaces_[surface_id]->vertex,sizeof(Vertex) * s->num_vertex);

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

const Vector3& Mesh::getMin(int surface_id)
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

const Vector3& Mesh::getMax(int surface_id)
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

const Vector3& Mesh::getCenter(int surface_id)
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

const std::string Mesh::getSurfaceName(int surface_id) const
{
	return surfaces_[surface_id]->name;	
}

int Mesh::getSurfaceId(const char *name) const
{
	for(int i = 0; i < surfaces_.size(); i++) 
	{
		if(!std::string(name).compare(surfaces_[i]->name))
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
			t->cv[0] = m.multiplyVec3(t->cv[0]);
			t->cv[1] = m.multiplyVec3(t->cv[1]);
			t->cv[2] = m.multiplyVec3(t->cv[2]);

			Vector3 normal;
			normal = Vector3::Cross(t->cv[1] - t->cv[0], t->cv[2] - t->cv[0]);
			normal.normalize();
			t->plane = Vector4(normal['x'], normal['y'], normal['z'], t->cv[0].negate().dot(normal));

			normal = Vector3::Cross(t->plane.getVector3(), t->cv[0] - t->cv[2]);	
			normal.normalize();
			t->c[0] = Vector4(normal['x'], normal['y'], normal['z'], t->cv[0].negate().dot(normal));

			normal = Vector3::Cross(t->plane.getVector3(), t->cv[1] - t->cv[0]);
			normal.normalize();
			t->c[1] = Vector4(normal['x'], normal['y'], normal['z'], t->cv[1].negate().dot(normal));

			normal = Vector3::Cross(t->plane.getVector3(), t->cv[2] - t->cv[1]);
			normal.normalize();
			t->c[2] = Vector4(normal['x'], normal['y'], normal['z'], t->cv[2].negate().dot(normal));
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
		t->cv[0] = m.multiplyVec3(t->cv[0]);
		t->cv[1] = m.multiplyVec3(t->cv[1]);
		t->cv[2] = m.multiplyVec3(t->cv[2]);

		Vector3 normal;
		normal = Vector3::Cross(t->cv[1] - t->cv[0], t->cv[2] - t->cv[0]);
		normal.normalize();
		t->plane = Vector4(normal['x'], normal['y'], normal['z'], t->cv[0].negate().dot(normal));

		normal = Vector3::Cross(t->plane.getVector3(), t->cv[0] - t->cv[2]);	
		normal.normalize();
		t->c[0] = Vector4(normal['x'], normal['y'], normal['z'], t->cv[0].negate().dot(normal));

		normal = Vector3::Cross(t->plane.getVector3(), t->cv[1] - t->cv[0]);
		normal.normalize();
		t->c[1] = Vector4(normal['x'], normal['y'], normal['z'], t->cv[1].negate().dot(normal));

		normal = Vector3::Cross(t->plane.getVector3(), t->cv[2] - t->cv[1]);
		normal.normalize();
		t->c[2] = Vector4(normal['x'], normal['y'], normal['z'], t->cv[2].negate().dot(normal));
	}
	create_mesh_bounds();
}