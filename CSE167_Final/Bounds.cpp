#include "Bounds.h"
#include <algorithm>
#include "BasicMath.h"

BBox::BBox() 
{
	clear();
}

BBox::BBox(const Vector3 &min,const Vector3 &max) 
{
	set(min,max);
}

BBox::~BBox() 
{
	
}


void BBox::clear() 
{
	//invalid BBox on clear
	min_ =  BasicMath::Vec3_INF;
	max_ =  BasicMath::Vec3_INF.negate();
}

void BBox::set(const Vector3 &min,const Vector3 &max)
{
	min_ = min_;
	max_ = max_;
}

void BBox::setTransform(const Matrix4 &transform) 
{
	Vector3 center = (min_ + max_).scale(0.5f);
	Vector3 axis = max_ - center;

	float x = std::abs(transform[0]) * axis['x'] + std::abs(transform[1]) * axis['y'] + std::abs(transform[2]) * axis['z'];
	float y = std::abs(transform[4]) * axis['x'] + std::abs(transform[5]) * axis['y'] + std::abs(transform[6]) * axis['z'];
	float z = std::abs(transform[8]) * axis['x'] + std::abs(transform[9]) * axis['y'] + std::abs(transform[10]) * axis['z'];

	center = transform.multiplyVec3(center);
	min_ = center - Vector3(x,y,z);
	max_ = center + Vector3(x,y,z);
}

void BBox::expand(const BBox &bb)
{
	if(bb.isValid()) 
	{
		if(isValid()) 
		{
			min_ = std::min(min_,bb.getMin());
			max_ = std::max(max_,bb.getMax());
		} 
		else 
		{
			min_ = bb.getMin();
			max_ = bb.getMax();
		}
	}
}

void BBox::expand(const Vector3 *verticesCoords,int num_vcoords)
{
	if(isValid()) 
	{
		BasicMath::VertexMinMax min_max 
			 = BasicMath::calculateMinMaxFromVertices(verticesCoords, num_vcoords);
		min_ = std::min(min_,min_max.first);
		max_ = std::max(max_,min_max.second);
	} 
	else 
	{
		BasicMath::VertexMinMax min_max 
			 = BasicMath::calculateMinMaxFromVertices(verticesCoords, num_vcoords);
		min_ = min_max.first;
		max_ = min_max.second;
	}
}

//////////////////////////////////////////////////////////////////////////
BSphere::BSphere()
{
	clear();
}

BSphere::BSphere(const Vector3 &center,float radius) 
{
	set(center,radius);
}

BSphere::~BSphere() 
{
	
}

void BSphere::clear() 
{
	center_ = Vector4(0, 0, 0, -1.0f);
}

void BSphere::set(const Vector3 &c,float r) 
{
	center_.set(c['x'], 'x');
	center_.set(c['y'], 'y');
	center_.set(c['z'], 'z');
	center_.set(r, 'w');
}

void BSphere::setTransform(const Matrix4 &transform) 
{
	float radius = center_['w'];
	
	Vector3 center_pos = Vector3(center_['x'], center_['y'], center_['z']);
	center_pos = transform.multiplyVec3(center_pos);
	center_.set(center_pos['x'], 'x');
	center_.set(center_pos['y'], 'y');
	center_.set(center_pos['z'], 'z');

	float x = transform[0] * transform[0] + transform[4] * transform[4] + transform[8] * transform[8];
	float y = transform[1] * transform[1] + transform[5] * transform[5] + transform[9] * transform[9];
	float z = transform[2] * transform[2] + transform[6] * transform[6] + transform[10] * transform[10];

	float scale = std::max(std::max(x,y), z);
	center_.set(radius * std::sqrt(scale), 'w');
}

void BSphere::expand(const Vector3 *vertexCoordArray,int num_vertices)
{
	if(isValid()) 
	{
		Vector3 center_pos(center_['x'], center_['y'], center_['z']);
		for(int i = 0; i < num_vertices; i++) 
		{
			Vector3 dir = vertexCoordArray[i].subtract(center_pos);
			float len = dir.getLength();
			if(len > center_['w']) 
			{
				float delta = (len - center_['w']) * 0.5f;
				center_pos = center_pos + dir.scale(delta / len);
				center_.set(center_pos['x'], 'x');
				center_.set(center_pos['y'], 'y');
				center_.set(center_pos['z'], 'z');

				center_.set(center_['w'] + delta, 'w');
			}
		}
	} 
	else 
	{
		BasicMath::VertexMinMax min_max = 
			BasicMath::calculateMinMaxFromVertices(vertexCoordArray, num_vertices);

		Vector3 center_pos(center_['x'], center_['y'], center_['z']);
		Vector3 new_center_pos = (min_max.first + min_max.second).scale(0.5f);
		center_.set(new_center_pos['x'], 'x');
		center_.set(new_center_pos['y'], 'y');
		center_.set(new_center_pos['z'], 'z');

		float radiusSquare = -MATH_INF;

		for(int i = 0; i < num_vertices; i++) 
		{
			float lenSquare = BasicMath::lengthSquare(vertexCoordArray[i] - center_pos);
			if(radiusSquare < lenSquare)
			{
				radiusSquare = lenSquare;
			}
		}

		center_.set((radiusSquare > 0.0f) ? std::sqrtf(radiusSquare) : -1.0f, 'w');
	}
}

void BSphere::expand(const BSphere &bs)
{
	if(bs.isValid()) 
	{
		if(isValid()) 
		{
			Vector3 dir = bs.getCenter().getVector3() - center_.getVector3();

			float len = dir.getLength();
			if(len > MATH_EPSILON) 
			{
				if(len + center_['w'] < bs.getCenter()['w']) 
				{
					center_ = bs.getCenter();
				} 
				else if(len + bs.getCenter()['w'] > center_['w']) 
				{
					Vector3 p0 = center_.getVector3() - dir.scale(center_['w'] / len);
					Vector3 p1 = bs.getCenter().getVector3() + dir.scale(bs.getCenter()['w'] / len);

					center_.setVector3((p0 + p1).scale(0.5f));
					center_.set((p1 - center_.getVector3()).getLength(), 'w');
				}
			}
			else 
			{
				if(center_['w'] < bs.getCenter()['w'])
				{
					center_.set(bs.getCenter()['w'], 'w');
				}
			}
		} 
		else 
		{
			center_ = bs.getCenter();
		}
	}
}