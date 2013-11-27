#pragma once
#include <vector>
#include "Vector3.h"
#include "Bounds.h"
#include "Node.h"

class WorldPosition;
class WorldNode
{
public:
	WorldNode();
	virtual ~WorldNode();

	void update();
	void render(const Vector3 &cam, float distance);

	// spatial collisions
	void getCollision(const BBox &bb,std::vector<Node*> &nodes) const;
	void getCollision(const BSphere &bs,std::vector<Node*> &nodes) const;
	void getCollision(const Vector3 &p0,const Vector3 &p1,std::vector<Node*> &nodes) const;

	// spatial intersections
	void getIntersection(std::vector<Node*> &nodes) const;
	void getIntersection(int type,std::vector<Node*> &nodes) const;

	void getIntersection(const Vector3 &camera,std::vector<Node*> &nodes) const;
	void getIntersection(const Vector3 &camera,int type,std::vector<Node*> &nodes) const;
	void getIntersection(const Vector3 &camera,float distance,std::vector<Node*> &nodes) const;
	void getIntersection(const Vector3 &camera,float distance,int type,std::vector<Node*> &nodes) const;

	void getIntersection(const BBox &bb,std::vector<Node*> &nodes) const;
	void getIntersection(const BBox &bb,int type,std::vector<Node*> &nodes) const;

	void getIntersection(const BSphere &bs,std::vector<Node*> &nodes) const;
	void getIntersection(const BSphere &bs,int type,std::vector<Node*> &nodes) const;

	void getIntersection(const BFrustum &bf,std::vector<Node*> &nodes) const;
	void getIntersection(const BFrustum &bf,int type,std::vector<Node*> &nodes) const;
	void getIntersection(const BFrustum &bf,const Vector3 &camera,float distance,std::vector<Node*> &nodes) const;
	void getIntersection(const BFrustum &bf,const Vector3 &camera,float distance,int type,std::vector<Node*> &nodes) const;

	void getIntersection(const Vector3 &p0,const Vector3 &p1,std::vector<Node*> &nodes) const;
	void getIntersection(const Vector3 &p0,const Vector3 &p1,int type,std::vector<Node*> &nodes) const;

	// positions
	static void AddPosition(WorldNode *&root,WorldPosition *position);
	static void RemovePosition(WorldNode *&root,WorldPosition *position);
	static void UpdatePosition(WorldNode *&root,WorldPosition *position);


private:
	friend class WorldPosition;

	inline bool isLeaf() const
	{
		return left_ == nullptr;
	}

	void update_bounds();
	WorldNode *parent_;					// parent node
	WorldNode *left_;					// left child
	WorldNode *right_;					// right child

	int num_positions_;					// number of positions
	WorldPosition *position_;			// world positions list

	bool need_update_;					// need update flag

	BBox bound_box_;					// node bound box
	BBox bound_threshold_;				// node bound threshold
	BSphere bound_sphere_;				// node bound sphere

};

class WorldPosition
{
public:
	explicit WorldPosition(Node *node);
	virtual ~WorldPosition();

private:
	// update bounds
	void update_bounds();

	// node positions
	void add_position(WorldPosition *position);
	void remove_position(WorldPosition *position);

	// best position node
	WorldNode *get_node(WorldPosition *position);

	// best separate plane
	Vector4 get_plane(std::vector<WorldPosition*> &positions);

	// all positions from the branch
	void get_positions(std::vector<WorldPosition*> &positions);

	Vector4 plane;							// splitting plane

};



class BSPTree
{
public:
	BSPTree(void);
	~BSPTree(void);

	void update();

private:
	WorldNode *world_root;
	WorldNode *obj_root;
	WorldNode *light_root;
};

