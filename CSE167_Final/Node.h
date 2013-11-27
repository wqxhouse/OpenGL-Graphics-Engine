#pragma once
#include "Factory.h"
#include "ManagedNode_Container.h"
#include <string>
#include <vector>
#include "Matrix4.h"
#include "Sector.h"
#include "Portal.h"

class Node
	: public Factory<Node>, 
	  public ManagedNode<Node>
{
public:
	enum
	{
		DUMMY, 

		WORLD, 
		WORLD_TRANSFORM,

		SOUND,

		LIGHT_DIRECTIONAL, 
		LIGHT_POINT, 
		LIGHT_SPOT, 

		OBJ_GEOMETRY, 
		OBJ_TERRAIN, 
		OBJ_GRASS, 
		OBJ_PARTICLES, 
		OBJ_BILLBOARD, 
		OBJ_WATER, 
		OBJ_SKY, 

		PLAYER_VIEWER, 
		PLAYER_ENEMY, 
		PLAYER_ACTOR, 

		NUM_NODES,
	};

	Node(int type);
	virtual ~Node(void);

	int node_id() const;
	
	//node factory functions
	static Node *CreateNode(const char *name, int type);

	inline void setFrame(int frame)
	{
		frame_ = frame;
	}

	inline void getFrame() const
	{
		return frame_;
	}

	inline int getType() const 
	{
		return type_;
	}

	inline bool isWorldNode() const
	{
		return (type >= WORLD && type <= WORLD_TRANSFORM);
	}

	inline bool isSoundNode() const
	{
		return (type == SOUND);
	}

	inline bool isLightNode() const
	{
		return (type >= LIGHT_DIRECTIONAL && type <= LIGHT_SPOT);
	}

	inline bool isObjectNode() const
	{
		return (type >= OBJ_GEOMETRY && type <= OBJ_SKY);
	}

	inline bool isPlayerNode() const
	{
		return (type >= PLAYER_VIEWER && type <= PLAYER_ACTOR);
	}

	inline void setEnabled(bool bEnable) 
	{
		enabled_ = bEnable;
	}

	inline void isEnabled() const
	{
		return enabled_;
	}

	inline void setShadow(bool bShadow)
	{
		shadow_ = bShadow;
	}

	inline bool isShadow() const
	{
		return shadow_;
	}

	inline void setName(const char *name)
	{
		name_ = name;
	}

	inline void getName() const
	{
		return name_;
	}

	void setParent();
	void getParent() const;
	void setParentWorld();

	void addChild();
	void removeChild();
	void addChildWorld();
	void removeChildWorld();

	int getChildCount() const;
	int findChildId() const;
	Node *getChildById() const;

	bool needUpdate() const;
	virtual void update(float spf);

	inline void setWorldSector(Sector *s)
	{
		sector_ = s;
	}

	inline Sector* getWorldSector() const
	{
		return sector_;
	}

	inline void getWorldPosition() const
	{
		return position_;
	}

	void setTransform(const Matrix4 &mat);
	inline const Matrix4 &getTransform() const
	{
		return transform_;
	}

	void setWorldTransform(const Matrix4 &mat);
	inline const Matrix4 &getWorldTransform()
	{
		return world_transform_;
	}
	

protected:
	// update world position
	void update_world_position();

	// default positions
	void update_world_sector_bound();
	void update_world_sector_center();
	void update_world_trigger_bound();
	void update_world_trigger_center();

	// update enabled
	virtual void update_enabled();

	// update position
	virtual void update_position();

	// update transformation
	virtual void update_transform();

private:

	friend class WorldSector;
	friend class WorldTrigger;
	friend class WorldPosition;

	int frame_;								// frame number

	char type_;								// node type

	bool enabled_;				// node enabled flag
	bool collider_;				// node collider flag
	bool hidden_;				// node hidden flag

	bool shadow_;				// node shadow flag

	std::string *name_;							// node name

	Node *parent_;							// parent node
	std::vector<Node*> childs_;				// child nodes

	Sector *sector_;					// world sector

	WorldPosition *position_;				// world position

	Matrix4 *transform_;						// transformation
	Matrix4 *world_transform_;					// world transformation
	Matrix4 *iworld_transform_;					// inverse world transformation
	
};

