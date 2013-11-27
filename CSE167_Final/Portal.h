#pragma once
#include "Node.h"
#include "Matrix4.h"
#include "Vector3.h"
#include "Bounds.h"
#include "PXml.h"

class Portal
	: public Node
{
public:
	Portal();
	Portal(float width,float height);
	virtual ~Portal();

	// size
	void setSize(float width,float height);
	float getWidth() const;
	float getHeight() const;

	// portal points
	int getNumPoints() const;
	const Vector3 *getPoints() const;

	// sectors
	void addWorldSector(Sector *sector);
	void removeWorldSector(Sector *sector);

	int getNumWorldSectors() const;
	Sector *getWorldSector(int num) const;

	// bounds
	virtual int isWorldBounds() const;
	virtual const BBox &getBoundBox() const;
	virtual const BSphere &getBoundSphere() const;

	// visualizer
	virtual void renderHandler();
	virtual void renderVisualizer();

	// load/save world
	virtual int loadWorld(const PXml *xml);


private:

	// world bounds
	void update_bounds();

	// update position
	virtual void update_position();

	// update transformation
	virtual void update_transform();

	float width;					// width
	float height;					// height

	Vector3 points[4];					// portal points

	BBox bound_box;				// bounding box
	BSphere bound_sphere;		// bounding sphere

	std::vector<Sector*> sectors;	// world sectors
};

