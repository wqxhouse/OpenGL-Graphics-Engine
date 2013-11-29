#include "MeshVBO.h"
#include "core.h"
#include "Vector3.h"


MeshVBO::MeshVBO(const Mesh *mesh) : Mesh(mesh) 
{
	for(int i = 0; i < getNumSurfaces(); i++) 
	{
		GLuint id;
		glGenBuffersARB(1,&id);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,id);
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,sizeof(Vertex) * getNumVertex(i),getVertex(i),GL_STATIC_DRAW_ARB);
		vbo_id_.push_back(id);
	}
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
}

MeshVBO::~MeshVBO() 
{
	for(int i = 0; i < getNumSurfaces(); i++)
	{
		glDeleteBuffersARB(1, &vbo_id_[i]);
	}
	vbo_id_.clear();
}

int MeshVBO::render(int pplShading,int surface_id) 
{
	int num_triangles = 0;
	if(pplShading) 
	{
		glEnableVertexAttribArrayARB(0);
		glEnableVertexAttribArrayARB(1);
		glEnableVertexAttribArrayARB(2);
		glEnableVertexAttribArrayARB(3);
		glEnableVertexAttribArrayARB(4);
		if(surface_id < 0) 
		{
			for(int i = 0; i < surfaces_.size(); i++) {
				glBindBufferARB(GL_ARRAY_BUFFER_ARB,vbo_id[i]);
				glVertexAttribPointerARB(0,3,GL_FLOAT,0,sizeof(Vertex),0);
				glVertexAttribPointerARB(1,3,GL_FLOAT,0,sizeof(Vertex),(void*)(sizeof(Vector3) * 1));
				glVertexAttribPointerARB(2,3,GL_FLOAT,0,sizeof(Vertex),(void*)(sizeof(Vector3) * 2));
				glVertexAttribPointerARB(3,3,GL_FLOAT,0,sizeof(Vertex),(void*)(sizeof(Vector3) * 3));
				glVertexAttribPointerARB(4,2,GL_FLOAT,0,sizeof(Vertex),(void*)(sizeof(Vector3) * 4));
				int *indices = surfaces[i]->indices;
				for(int j = 0; j < surfaces[i]->num_strips; j++) 
				{
					glDrawElements(GL_TRIANGLE_STRIP,indices[0],GL_UNSIGNED_INT,indices + 1);
					indices += indices[0] + 1;
				}
				num_triangles += getNumTriangles(i);
			}
		} 
		else 
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,vbo_id[surface_id]);
			glVertexAttribPointerARB(0,3,GL_FLOAT,0,sizeof(Vertex),0);
			glVertexAttribPointerARB(1,3,GL_FLOAT,0,sizeof(Vertex),(void*)(sizeof(Vector3) * 1));
			glVertexAttribPointerARB(2,3,GL_FLOAT,0,sizeof(Vertex),(void*)(sizeof(Vector3) * 2));
			glVertexAttribPointerARB(3,3,GL_FLOAT,0,sizeof(Vertex),(void*)(sizeof(Vector3) * 3));
			glVertexAttribPointerARB(4,2,GL_FLOAT,0,sizeof(Vertex),(void*)(sizeof(Vector3) * 4));
			int *indices = surfaces[surface_id]->indices;
			for(int i = 0; i < surfaces[surface_id]->num_strips; i++) 
			{
				glDrawElements(GL_TRIANGLE_STRIP,indices[0],GL_UNSIGNED_INT,indices + 1);
				indices += indices[0] + 1;
			}
			num_triangles += getNumTriangles(surface_id);
		}

		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glDisableVertexAttribArrayARB(4);
		glDisableVertexAttribArrayARB(3);
		glDisableVertexAttribArrayARB(2);
		glDisableVertexAttribArrayARB(1);
		glDisableVertexAttribArrayARB(0);
	} 
	else 
	{
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		if(surface_id < 0) 
		{
			for(int i = 0; i < surfaces_.size(); i++) 
			{
				glBindBufferARB(GL_ARRAY_BUFFER_ARB,vbo_id[i]);
				glVertexPointer(3,GL_FLOAT,sizeof(Vertex),0);
				glNormalPointer(GL_FLOAT,sizeof(Vertex),(void*)(sizeof(Vector3) * 1));
				glTexCoordPointer(2,GL_FLOAT,sizeof(Vertex),(void*)(sizeof(Vector3) * 4));
				int *indices = surfaces[i]->indices;
				for(int j = 0; j < surfaces[i]->num_strips; j++) 
				{
					glDrawElements(GL_TRIANGLE_STRIP,indices[0],GL_UNSIGNED_INT,indices + 1);
					indices += indices[0] + 1;
				}
				num_triangles += getNumTriangles(i);
			}
		} 
		else 
		{
			glBindBufferARB(GL_ARRAY_BUFFER_ARB,vbo_id[surface_id]);
			glVertexPointer(3,GL_FLOAT,sizeof(Vertex),0);
			glNormalPointer(GL_FLOAT,sizeof(Vertex),(void*)(sizeof(Vector3) * 1));
			glTexCoordPointer(2,GL_FLOAT,sizeof(Vertex),(void*)(sizeof(Vector3) * 4));
			int *indices = surfaces[surface_id]->indices;
			for(int i = 0; i < surfaces[surface_id]->num_strips; i++) 
			{
				glDrawElements(GL_TRIANGLE_STRIP,indices[0],GL_UNSIGNED_INT,indices + 1);
				indices += indices[0] + 1;
			}
			num_triangles += getNumTriangles(surface_id);
		}
		glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	return num_triangles;
}