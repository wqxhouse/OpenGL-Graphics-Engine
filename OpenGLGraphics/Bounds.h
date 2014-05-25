#pragma once
#include "Vector4.h"
#include "Matrix4.h"

class BSphere;
class BBox
{
public:
	BBox();
	BBox(const Vector3 &min,const Vector3 &max);
	~BBox();

	void clear();

	void set(const Vector3 &min,const Vector3 &max);
	void setTransform(const Matrix4 &transform);

	//// compare
	//int compare(const BBox &bb) const;

	// expand
	//void expand(const Vector3 &point);
	void expand(const Vector3 *verticesCoords,int num_vcoords);
	//void expand(const BSphere &bs);
	void expand(const BBox &bb);

	//// inside points
	//int inside(const Vector3 &point) const;
	//int inside(const Vector3 &point,float radius) const;
	//int inside(const Vector3 &min,const Vector3 &max) const;

	//int insideValid(const Vector3 &point) const;
	//int insideValid(const Vector3 &point,float radius) const;
	//int insideValid(const Vector3 &min,const Vector3 &max) const;

	//// inside bounds
	//int inside(const BSphere &bs) const;
	//int inside(const BBox &bb) const;

	//int insideValid(const BSphere &bs) const;
	//int insideValid(const BBox &bb) const;

	//int insideAll(const BSphere &bs) const;
	//int insideAll(const BBox &bb) const;

	//// inside cube
	//int insideCube(int face,const Vector3 &offset) const;

	//// intersections
	//int rayIntersection(const Vector3 &p,const Vector3 &dir) const;
	//int irayIntersection(const Vector3 &p,const Vector3 &idir) const;
	//int getIntersection(const Vector3 &p0,const Vector3 &p1) const;

	//int rayIntersectionValid(const Vector3 &p,const Vector3 &dir) const;
	//int irayIntersectionValid(const Vector3 &p,const Vector3 &idir) const;
	//int getIntersectionValid(const Vector3 &p0,const Vector3 &p1) const;

	//// distance
	//float distance(const Vector3 &p) const;
	//float distanceValid(const Vector3 &p) const;

	//// parameters
	inline int isValid() const { return (min_['x'] <= max_['x']); }
	inline const Vector3 &getMin() const { return min_; }
	inline const Vector3 &getMax() const { return max_; }
	//void getPoints(Vector3 *points,int num_points) const;
	//void getPlanes(Vector4 *planes,int num_planes) const;

private:

	Vector3 min_;		// bounding box minimum
	Vector3 max_;		// bounding box maximum
};

class BSphere
{
public:
	BSphere();
	BSphere(const Vector3 &center, float radius);
	~BSphere();

	void clear();

	void set(const Vector3 &center,float radius);
	inline void setCenter(const Vector3 &center)
	{
		center_.set(center['x'], 'x');
		center_.set(center['y'], 'y');
		center_.set(center['z'], 'z');
	}

	inline void setRadius(float radius)
	{
		center_.set(radius, 'w');
	}
	void setTransform(const Matrix4 &transform);

	//// compare
	//int compare(const BSphere &bs) const;

	//// expand
	void expand(const Vector3 *vertexCoordArray,int num_vertices);
	void expand(const BSphere &bs);

	//// radius expand
	//void expandRadius(const Vector3 &point);
	//void expandRadius(const Vector3 *points, int num_points);
	//void expandRadius(const BSphere &bs);
	//void expandRadius(const BBox &bb);

	//// inside points
	//int inside(const Vector3 &point) const;
	//int inside(const Vector3 &point,float radius) const;
	//int inside(const Vector3 &min,const Vector3 &max) const;

	//int insideValid(const Vector3 &point) const;
	//int insideValid(const Vector3 &point,float radius) const;
	//int insideValid(const Vector3 &min,const Vector3 &max) const;

	//// inside bounds
	//int inside(const BSphere &bs) const;
	//int inside(const BBox &bb) const;

	//int insideValid(const BSphere &bs) const;
	//int insideValid(const BBox &bb) const;

	//int insideAll(const BSphere &bs) const;
	//int insideAll(const BBox &bb) const;

	//// intersections
	//int rayIntersection(const Vector3 &p,const Vector3 &dir) const;
	//int getIntersection(const Vector3 &p0,const Vector3 &p1) const;

	//int rayIntersectionValid(const Vector3 &p,const Vector3 &dir) const;
	//int getIntersectionValid(const Vector3 &p0,const Vector3 &p1) const;

	//// distance
	//float distance(const Vector3 &p) const;
	//float distanceValid(const Vector3 &p) const;

	// parameters
	inline int isValid() const { return (center_['w'] > 0.0f); }
	inline const Vector4 &getCenter() const { return center_; }
	inline float getRadius() const { return center_['w']; }

private:

	Vector4 center_;	// bounding sphere center and radius
};
