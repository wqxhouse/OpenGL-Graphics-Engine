#pragma once

#include <vector>
#include <fstream>
#include "Bounds.h"
#include "Vector4.h"
#include "Vector3.h"

class Mesh;
class Material;
class Object;
class OGeometry;

/*
 */
class Node 
{
public:
	
	Node();
	~Node();
	
	void create(Mesh *mesh);
	
	void bindMaterial(const char *name, Material *material);
	void render();
	
	enum 
	{
		TRIANGLES_PER_NODE = 1024,
	};
	
	BBox bbox_;
	BSphere bsphere_;
	
	Node *left_;
	Node *right_;
	
	OGeometry *object_;	// object
};

/*
 */
class Portal {
public:
	
	Portal();
	~Portal();
	
	void create(Mesh *mesh,int surface_id);
	void getScissor(int *scissor);
	void render();
	
	BSphere bsphere_;
	std::vector<int> sectors_;
	
	Vector3 points_[4];

	int frame_;
};

/*
 */
class Sector {
public:
	
	Sector();
	~Sector();
	
	void create(Mesh *mesh,int s);
	void getNodeObjects(Node *node);
	void create();
	
	int inside(const Vector3 &point);
	int inside(Portal *portal);
	int inside(const BSphere &bsphere_);
	int inside(Mesh *mesh, int surface_id);
	
	void addObject(Object *object);
	void removeObject(Object *object);
	
	void bindMaterial(const char *name,Material *material);
	void render(Portal *portal = nullptr);
	
	void saveState();
	void restoreState(int frame);
	
	enum 
	{
		NUM_OBJECTS = 256,
	};

	Node *root;						// binary tree
	
	BSphere bsphere_;
	
	std::vector<Vector4> planes_;	//bounding planes
	
	std::vector<int> portals_;
	std::vector<Object*> objects_;
	std::vector<Object*> node_objects_;	
	std::vector<Object*> visible_objects_;
	
	Portal *portal_;					// sector is visible through this portal
	int frame_;

	std::vector<Object*> old_visible_objects_;
	Portal *old_portal_;
	int old_frame_;
};

/*
 */
class BSPTree 
{
public:
	
	BSPTree();
	~BSPTree();
	
	void load(const char *name);
	void save(const char *name);
	
	void bindMaterial(const char *name, Material *material);
	void render();
	
	void saveState();
	void restoreState(int frame);
	
	static std::vector<Portal> portals_;
	static std::vector<Sector> sectors_;

	static std::vector<Sector*> visible_sectors_;
	static std::vector<Sector*> old_visible_sectors_;
};
