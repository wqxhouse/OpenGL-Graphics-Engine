#pragma once
class Node
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

		OBJ_MESH, 
		OBJ_TERRAIN, 
		OBJ_GRASS, 
		OBJ_PARTICLES, 
		OBJ_BILLBOARD, 
		OBJ_WATER, 
		OBJ_SKY, 

		PLAYER_VIEWER, 
		

	};

	Node(void);
	virtual ~Node(void);

	int node_id() const;

	
};

