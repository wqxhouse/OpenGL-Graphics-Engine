#pragma once
#include <vector>

#include "BSPTree.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Material.h"
#include "Node.h"
#include "PXml.h"

//class Object
//	: public Node
//{
//	Object(int type);
//	virtual ~Object();
//
//	virtual void update(float spf);	// update function
//	void updatePos(const Vector3 &p);		// update position
//
//	int bindMaterial(const char *name, Material *material);
//
//	enum 
//	{
//		RENDER_ALL = 0,
//		RENDER_OPACITY,
//		RENDER_TRANSPARENT
//	};
//
//	virtual int render(int t = RENDER_ALL,int s = -1) = 0;
//
//	virtual void findSilhouette(const Vector4 &light,int s = -1) = 0;
//	virtual int getNumIntersections(const Vector3 &line0,const Vector3 &line1,int s = -1) = 0;
//	virtual int renderShadowVolume(int s = -1)  = 0;
//
//	virtual int intersection(const Vector3 &line0,const Vector3 &line1, Vector3 *point,Vector3 *normal, int s = -1) = 0;
//
//	virtual int getNumSurfaces() = 0;
//	virtual const char *getSurfaceName(int s) = 0;
//	virtual int getSurface(const char *name) = 0;
//
//	virtual const Vector3 &getMin(int s = -1) = 0;
//	virtual const Vector3 &getMax(int s = -1) = 0;
//	virtual const Vector3 &getCenter(int s = -1) = 0;
//	virtual float getRadius(int s = -1) = 0;
//
//	//void setRigidBody(RigidBody *rigidbody);
//
//	void setShadows(int shadows);
//
//	virtual void set_position(const Vector3 &p);	// set position
//	virtual void set_transform(const Matrix4 &m);	// set transformation
//
//	void enable();				// enable transformation
//	void disable();				// disable
//
//	enum {
//		OBJECT_MESH = 0,
//		OBJECT_SKINNEDMESH,
//		OBJECT_PARTICLES
//	};
//
//	int type;					// type of the object
//
//	Vector3 pos;				// position of the object
//	//TODO: potential expansion of the position to allow bezier path moving
//
//	//RigidBody *rigidbody;		// rigidbody dynamic
//
//	int is_identity;
//	Matrix4 transform;
//	Matrix4 itransform;
//
//	Matrix4 prev_modelview;			// save old matrixes
//	Matrix4 prev_imodelview;
//	Matrix4 prev_transform;
//	Matrix4 prev_itransform;
//
//	std::vector<Material*> materials_;		// all materials
//
//	//TODO: add transparency property
//
//	int shadows_;
//
//	float time_;					// object time
//	int frame_;
//};

class Object
	: public Node
{
	Object(int type);
	virtual ~Object();

	// surfaces
	INLINE int getNumSurfaces() const { return surfaces.size(); }
	virtual int findSurface(const char *name) const;
	virtual const char *getSurfaceName(int surface) const = 0;

	// compares
	virtual int getResource(int surface);
	virtual int getOrder(const Vector3 &camera,int surface);
	virtual int getSequence(const Vector3 &camera,int surface);
	virtual float getOpacityDistance(const Vector3 &camera,int surface);
	virtual float getTransparentDistance(const Vector3 &camera,int surface);

	// enabled
	void setEnabled(int enabled);
	int isEnabled() const;

	void setEnabled(int enable,int surface);
	int isEnabled(int surface) const;

	// hidden
	void setHidden(int hide,int surface);
	int isHidden(int surface) const;

	// parent
	void setParent(Node *parent);
	Node *getParent() const;

	void setParent(int parent,int surface);
	int getParent(int surface) const;

	// options
	void setDecal(int enable,int surface);
	int getDecal(int surface) const;

	void setCastShadow(int enable,int surface);
	int getCastShadow(int surface) const;

	void setReceiveShadow(int enable,int surface);
	int getReceiveShadow(int surface) const;

	void setCastWorldShadow(int enable,int surface);
	int getCastWorldShadow(int surface) const;

	void setReceiveWorldShadow(int enable,int surface);
	int getReceiveWorldShadow(int surface) const;

	void setIntersection(int enable,int surface);
	int getIntersection(int surface) const;

	void setCollision(int enable,int surface);
	int getCollision(int surface) const;

	void setIntersectionMask(int mask,int surface);
	int getIntersectionMask(int surface) const;

	void setCollisionMask(int mask,int surface);
	int getCollisionMask(int surface) const;

	void setReflectionMask(int mask,int surface);
	int getReflectionMask(int surface) const;

	// level of detail
	virtual int hasLods() const;
	void updateLods(const Vector3 &camera);
	void updateLods(const Vector3 &camera,int surface);

	void setMinParent(int parent,int surface);
	int getMinParent(int surface) const;

	void setMinVisibleDistance(float distance,int surface);
	float getMinVisibleDistance(int surface) const;

	void setMinFadeDistance(float distance,int surface);
	float getMinFadeDistance(int surface) const;

	void setMaxParent(int parent,int surface);
	int getMaxParent(int surface) const;

	void setMaxVisibleDistance(float distance,int surface);
	float getMaxVisibleDistance(int surface) const;

	void setMaxFadeDistance(float distance,int surface);
	float getMaxFadeDistance(int surface) const;

	int isVisible(int surface) const;
	int isFaded(int surface) const;

	// render distance
	void setRenderDistance(float distance);
	INLINE float getRenderDistance() const { return render_distance; }

	// materials
	void setMaterial(const char *name,int surface);
	void setMaterial(const char *name,const char *pattern);
	Material *getMaterial(int surface) const;
	const char *getMaterialName(int surface) const;

	// world sector
	void setWorldSector(WorldSector *sector);
	WorldSector *getWorldSector() const;

	void setWorldSector(WorldSector *sector,int surface);
	WorldSector *getWorldSector(int surface) const;

	// object surfaces
	ObjectSurface *getObjectSurface(int surface);

	void setObjectSurfaceFrame(int frame,int surface);
	int getObjectSurfaceFrame(int surface) const;

	// object decals
	void addObjectDecal(ObjectDecal *decal,int surface);
	void removeObjectDecal(ObjectDecal *decal,int surface);

	int getNumObjectDecals(int surface) const;
	ObjectDecal *getObjectDecal(int num,int surface) const;

	void updateObjectDecals() const;

	// object impostor
	void setObjectImpostor(ObjectImpostor *i);
	INLINE ObjectImpostor *getObjectImpostor() const { return impostor; }

	// collision
	virtual int getCollision(const BBox &bb,Vector<int> &csurfaces);
	virtual int getCollision(const BSphere &bs,Vector<int> &csurfaces);
	virtual int getCollision(const BoundFrustum &bf,Vector<int> &csurfaces);
	virtual int getCollision(const Vector3 &p0,const Vector3 &p1,Vector<int> &csurfaces);

	// line intersection
	virtual int getIntersection(const Vector3 &p0,const Vector3 &p1,int surface);
	virtual int getIntersection(const Vector3 &p0,const Vector3 &p1,Vector3 &ret_point,Vector4 &ret_plane,int &ret_triangle,int surface);
	virtual int getIntersection(const Vector3 &p0,const Vector3 &p1,Vector3 &ret_point,Vector3 &ret_normal,Vector4 &ret_texcoord,int surface);
	int getIntersection(const Vector3 &p0,const Vector3 &p1,int mask,Vector3 &ret_point,Vector4 &ret_plane,int &ret_triangle,int &ret_surface);
	int getIntersection(const Vector3 &p0,const Vector3 &p1,int mask,Vector3 &ret_point,Vector3 &ret_normal,Vector4 &ret_texcoord,int &ret_surface);

	// random point
	virtual int getRandomPoint(Vector3 &ret_point,Vector3 &ret_normal,Vector3 &ret_velocity,int surface);

	// surface info
	virtual int getNumTriangles(int surface) const;

	// surface bounds
	virtual const BBox &getBBox(int surface) const = 0;
	virtual const BSphere &getBSphere(int surface) const = 0;

	// surface world bounds
	virtual const BBox &getWorldBBox(int surface) const;
	virtual const BSphere &getWorldBSphere(int surface) const;

	// bounds
	virtual const BBox &getBBox() const = 0;
	virtual const BSphere &getBSphere() const = 0;

	// world bounds
	virtual const BBox &getWorldBBox() const;
	virtual const BSphere &getWorldBSphere() const;

	// visible distances
	virtual float getMinDistance() const;
	virtual float getMaxDistance() const;

	// create
	virtual int hasCreate() const;
	virtual ObjectSurface *create(ObjectSurface *surface);

	// render
	virtual ObjectSurface *render(int pass,ObjectSurface *surface);
	virtual ObjectSurface *render(int pass,const Light *light,ObjectSurface *surface);
	virtual ObjectSurface *render(int pass,const BSphere &bs,ObjectSurface *surface);

	// render shadow map
	virtual int hasShadow() const;
	virtual ObjectSurface *renderShadow(int pass,ObjectSurface *surface);
	virtual ObjectSurface *renderShadow(int pass,const Light *light,ObjectSurface *surface);
	virtual ObjectSurface *renderShadow(int pass,const BSphere &bs,ObjectSurface *surface);

	// render velocity
	virtual int hasVelocity() const;
	virtual ObjectSurface *renderVelocity(int pass,ObjectSurface *surface);

	// render query
	virtual int hasQuery() const;
	virtual void renderQuery();

	// clone object
	virtual Object *clone() const;

	// load/save world
	virtual int loadWorld(const PXml *xml);

protected:

	// surfaces
	virtual int get_num_surfaces() const = 0;

	// update level of detal
	void update_lods();

	// update surfaces
	void update_surfaces();

	// default positions
	void update_world_sector_bound();
	void update_world_sector_center();

	// update position
	virtual void update_position();

	// update transformation
	virtual void update_transform();
};