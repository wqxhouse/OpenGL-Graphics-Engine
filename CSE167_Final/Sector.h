#pragma once
#include "Node.h"
#include <vector>
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Bounds.h"
#include "Object.h"
#include "PXml.h"

class Sector
	: public Node
{
	Sector();
	Sector(const Vector3 &size);
	virtual ~Sector();

	// size
	void setSize(const Vector3 &size);
	const Vector3 &getSize() const;

	// inside
	int inside(const Vector3 &point) const;
	int inside(const Vector3 *points,int num_points) const;
	int inside(const BBox &bb) const;
	int inside(const BBox &bb,const Matrix4 &transform) const;

	// nodes
	void addNode(Node *node);
	void removeNode(Node *node);

	int getNumNodes() const;
	Node *getNode(int num) const;

	// portals
	void addPortal(Portal *portal);
	void removePortal(Portal *portal);
	void sortPortals(const Vector3 &camera);

	int getNumPortals() const;
	Portal *getPortal(int num) const;

	// object surfaces
	void addObjectSurface(Object *object,int surface);
	void removeObjectSurface(Object *object,int surface);

	int getNumObjectSurfaces() const;
	Object *getObjectSurface(int num,int &surface) const;

	// trace nodes
	void addTraceNode(Node *node);
	void clearTraceNodes();

	int getNumTraceNodes() const;
	Node *getTraceNode(int num) const;

	// trace portals
	void addTracePortal(Portal *portal);
	void clearTracePortals();

	int getNumTracePortals() const;
	Portal *getTracePortal(int num) const;

	void saveTracePortals();
	void restoreTracePortals();

	// tracing
	int getTraceOutdoor(const BSphere &bs,int depth) const;
	int getTracePath(const BSphere &bs,const Portal *portal) const;

	// bounds
	virtual int isWorldBounds() const;
	virtual const BBox &getBoundBox() const;
	virtual const BSphere &getBoundSphere() const;


	// load/save world
	virtual int loadWorld(const PXml *xml);

private:

	friend struct SectorDistanceCompare;

	// tracing
	int get_trace_outdoor(const BSphere &bs, std::vector<Sector*> &sectors, std::vector<Portal*> &portals,int depth) const;

	// world bounds
	void update_bounds();

	// update position
	virtual void update_position();

	// update transformation
	virtual void update_transform();

	// copy world
	Sector *copy(Sector *node) const;

	Vector3 size;								// size

	Vector3 points[8];							// sector points
	Vector4 planes[6];							// sector planes

	BBox bound_box;						// bounding box
	BSphere bound_sphere;				// bounding sphere

	struct PortalData {
		Portal *portal;				// portal pointer
		float distance;						// portal distance
	};

	struct ObjectSurface {
		Object *object;						// object pointer
		int surface;						// surface number
	};

	std::vector<Node*> nodes;					// nodes
	std::vector<PortalData> portals;					// world portals
	std::vector<ObjectSurface> object_surfaces;	// object surfaces

	std::vector<Node*> trace_nodes;				// trace nodes
	std::vector<Portal*> trace_portals;		// trace portals
	std::vector<Portal*> old_trace_portals;	// old trace portals
};