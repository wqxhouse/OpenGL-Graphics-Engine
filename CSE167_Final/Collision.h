#pragma once

#include "BSPTree.h"
#include "BSPTree.h"
#include "Position.h"

class Object;

class Collision 
{

public:
	
	Collision();
	~Collision();
	
	int collide(Object *object,const Vector3 &pos,float radius);
	int collide(Object *object,const Position &pos,float radius);
	
	int collide(Object *object);
	
	void sort();
	
	enum {
		NUM_CONTACTS = 32,
		NUM_OBJECTS = 16,
	};
	
	struct Contact {
		Object *object;
		Material *material;
		Vector3 point;
		Vector3 normal;
		float depth;
	};
	
	int num_contacts;
	Contact *contacts;
	
	int num_objects;
	Object **objects;
	
protected:
	
	int addContact(Object *object,Material *material,const Vector3 &point,const Vector3 &normal,float depth,int min_depth = 0);
	void collideObjectSphere(Object *object,const Vector3 &pos,float radius);
	void collideObjectMesh(Object *object);
	
	enum {
		NUM_TRIANGLES = 1024,
		NUM_SURFACES = 16,
	};
	
	struct Triangle
	{
		Vector3 v[3];			// vertexes
		Vector4 plane;			// plane
		Vector4 c[3];			// fast point in triangle
	};
	
	struct Surface
	{
		int num_triangles;		// triangles
		Triangle *triangles;
		Vector3 center;			// bound sphere
		float radius;
		Vector3 min;				// bound box
		Vector3 max;
	};
	
	static int counter;
	
	static Position position;
	
	static int num_surfaces;
	static Surface *surfaces;
};

