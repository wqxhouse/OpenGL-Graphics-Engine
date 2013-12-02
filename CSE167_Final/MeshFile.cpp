#include "MeshFileObj.h"
#include "assert.h"
#include "Vector4.h"
#include "string.h"

MeshFileOBJ::MeshFileOBJ()
	: vertices(), 
	  surfaces(), 
	  texcoords(), 
	  file()
{

}

MeshFileOBJ::~MeshFileOBJ()
{

}

int MeshFileOBJ::load(const char *name) 
{
	file.open(name, std::ios::binary | std::ios::in);
	if(file.fail())
	{
		assert(false && "MeshFileOBJ::load(): can't open file\n");
		return 0;
	}
	
	// default surface
	surfaces.push_back(Surface());
	Surface *surface = &surfaces.back();
	surface->name = name;

	// parse lines
	char buf[1024];
	while(file.eof() == 0) {
		file.getline(buf, sizeof(buf));
		const char *s = buf;
		s += skip_spaces(s);
		
		// vertices
		if(!strncmp(s,"v ",2)) 
		{
			float x, y, z;
			sscanf(s + 2,"%f %f %f\n", &x, &y, &z);
			vertices.push_back(Vector3(x, y, z));
		}
		
		// texture coordinates
		else if(!strncmp(s,"vt ",3))
		{
			float u, v;
			sscanf(s + 3,"%f %f\n", &u, &v);
			texcoords.push_back(Vector3(u, v, 0));
		}
		
		// normals
		else if(!strncmp(s,"vn ",3)) 
		{
			float x, y, z;
			sscanf(s + 3,"%f %f %f\n", &x, &y, &z);
			normals.push_back(Vector3(x, y, z));
		}
		
		// faces
		else if(!strncmp(s,"f ",2)) 
		{
			s += 2;
			Vertex v[4];
			int num_vertex = 0;
			memset(v,0,sizeof(v));
			for(int i = 0; i < 4; i++) 
			{
				int ret = read_vertex(s,v[i]);
				if(ret == 0) break;
				num_vertex++;
				s += ret;
			}
			if(num_vertex == 3)
			{
				surface->vertex.push_back(v[0]);
				surface->vertex.push_back(v[1]);
				surface->vertex.push_back(v[2]);
			} 
			else if(num_vertex == 4) 
			{
				surface->vertex.push_back(v[0]);
				surface->vertex.push_back(v[1]);
				surface->vertex.push_back(v[2]);
				surface->vertex.push_back(v[2]);
				surface->vertex.push_back(v[3]);
				surface->vertex.push_back(v[0]);
			}
		}
		
		// groups
		else if(!strncmp(s,"g ",2)) 
		{
			s += 2;
			s += skip_spaces(s);
			if(surface->vertex.size()) 
			{
				surfaces.push_back(Surface());
				surface = &surfaces.back();
			}
			surface->name = s;
		}
	}
	
	// smooth normals
	if(normals.size() == 0) {
		calculate_normals();
	}
	
	file.close();
	
	return 1;
}

/*
 */
int MeshFileOBJ::skip_spaces(const char *src) 
{
	const char *s = src;
	while(*s && strchr(" \t\n\r",*s) != NULL) s++;
	
	return (int)(s - src);
}

int MeshFileOBJ::read_index(const char *src,int &index) 
{
	const char *s = src;
	char buf[1024];
	char *d = buf;
	while(*s && *s != '/' && strchr(" \t\n\r",*s) == NULL) *d++ = *s++;
	*d = '\0';
	index = atoi(buf) - 1;
	
	return (int)(s - src);
}

int MeshFileOBJ::read_vertex(const char *src,Vertex &v) 
{
	const char *s = src;
	s += skip_spaces(s);
	s += read_index(s,v.v);
	if(*s == '/') {
		s++;
		if(*s == '/') {
			s++;
			s += read_index(s,v.vn);
		} else {
			s += read_index(s,v.vt);
			if(*s == '/') {
				s++;
				s += read_index(s,v.vn);
			}
		}
	}
	s += skip_spaces(s);
	if(v.v < 0) v.v = vertices.size() + v.v;
	if(v.vt < 0) v.vt = texcoords.size() + v.vt;
	if(v.vn < 0) v.vn = normals.size() + v.vn;
	
	return (int)(s - src);
}

/*
 */
void MeshFileOBJ::calculate_normals() 
{
	normals.resize(vertices.size());
	for(int i = 0; i < normals.size(); i++) 
	{
		normals[i] = Vector3(0, 0, 0);
	}
	
	for(int i = 0; i < surfaces.size(); i++) 
	{
		Surface *s = &surfaces[i];
		for(int j = 0; j < s->vertex.size(); j += 3) 
		{
			Vertex *v0 = &s->vertex[j + 0];
			Vertex *v1 = &s->vertex[j + 1];
			Vertex *v2 = &s->vertex[j + 2];
			v0->vn = v0->v;
			v1->vn = v1->v;
			v2->vn = v2->v;
			Vector3 normal = Vector3::Normalize(Vector3::Cross(vertices[v1->v] - vertices[v0->v],vertices[v2->v] - vertices[v0->v]));
			normals[v0->vn] = normals[v0->vn].add(normal);
			normals[v1->vn] = normals[v1->vn].add(normal);
			normals[v2->vn] = normals[v2->vn].add(normal);
		}
	}
	
	for(int i = 0; i < normals.size(); i++) 
	{
		normals[i].normalize();
	}
}

/*
 */
int MeshFileOBJ::getNumSurfaces() const
{ 
	
	int num_surfaces = 0;
	for(int i = 0; i < surfaces.size(); i++) 
	{
		const Surface &s = surfaces[i];
		if(s.vertex.size() == 0) continue;
		num_surfaces++;
	}
	
	return num_surfaces;
}

const char *MeshFileOBJ::getSurfaceName(int surface) const
{
	int num_surfaces = 0;
	for(int i = 0; i < surfaces.size(); i++) 
	{
		const Surface &s = surfaces[i];
		if(s.vertex.size() == 0) continue;
		if(num_surfaces++ == surface) 
		{
			return s.name.c_str();
		}
	}
	
	return NULL;
}

/*
 */
int MeshFileOBJ::getNumVertex(int surface) const
{
	
	int num_surfaces = 0;
	for(int i = 0; i < surfaces.size(); i++) 
	{
		const Surface &s = surfaces[i];
		if(s.vertex.size() == 0) continue;
		if(num_surfaces++ == surface) 
		{
			return s.vertex.size();
		}
	}
	
	return 0;
}

Mesh::Vertex *MeshFileOBJ::getVertex(int surface) const
{
	Mesh::Vertex *vertex = nullptr;
	
	int num_surfaces = 0;
	for(int i = 0; i < surfaces.size(); i++) 
	{
		const Surface &s = surfaces[i];
		if(s.vertex.size() == 0)
		{
			continue;
		}
		if(num_surfaces++ == surface) 
		{
			Mesh::Vertex *vertex = new Mesh::Vertex[s.vertex.size()];
			for(int j = 0; j < s.vertex.size(); j++) 
			{
				vertex[j].xyz = vertices[s.vertex[j].v];
				if(normals.size())
				{
					vertex[j].normal = normals[s.vertex[j].vn];
				}
				if(texcoords.size())
				{
					vertex[j].texcoord = 
						Vector3(texcoords[s.vertex[j].vt]['x'],
						        1.0f - texcoords[s.vertex[j].vt]['y'], 
								0.0f);
				}
			}
			return vertex;
		}
	}
	
	return vertex;
}
