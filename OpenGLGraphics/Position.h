#pragma once
#include <vector>
#include "Vector3.h"
#include "Matrix4.h"
#include "Bounds.h"

class Object;
class Spline;
class Expression;

class Position
{
public:
	Position();
	Position(const Vector3 &posCoord);
	~Position();
	
	void find(int sector, float r);
	
	void setSpline(Spline *spline);
	void setExpression(Expression *expression);
	
	void setRadius(float radius);
	void setPosition(const Vector3 &v);
	void copyPosition(const Position &pos);
	
	void update(float time,Matrix4 &transform);
	void update(float time,Object *object = nullptr);
	
	Matrix4 to_matrix(float time);
	
	Spline *spline_;
	Expression *expression_;

	inline Vector3 getPosCoord() const
	{
		return bsphere_.getCenter().getVector3();
	}

	inline float getBoundingRadius() const
	{
		return bsphere_.getRadius();
	}

	Position &operator=(const Position &pos);
	Position &operator=(const Vector3 &pos);
	
	enum 
	{
		NUM_SECTORS = 32,
	};
	
	int in_sector_id_;
	std::vector<int> sectors_;
	BSphere bsphere_;
};

/*
 */
class Spline 
{
public:
	Spline(const char *name,float speed,int close,int follow);
	Spline(const Spline &spline);
	~Spline();
	
	Matrix4 to_matrix(float time);
	
protected:
	int num_;
	Vector3 *params_;
	float speed_;
	float length_;
	int follow_;
};

