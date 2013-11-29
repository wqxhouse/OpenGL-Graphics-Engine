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
	
	Position(const Vector3 &posCoord);
	~Position();
	
	Position &operator=(const Position &pos);
	void find(int sector,float r);
	
	void setSpline(Spline *spline);
	void setExpression(Expression *expression);
	
	void setRadius(float radius);
	
	void update(float time,Matrix4 &transform);
	void update(float time,Object *object = nullptr);
	
	Matrix4 to_matrix(float time);
	
	Spline *spline;
	Expression *expression;
	inline Vector3 getPosCoord()
	{
		return pos_;
	}

	
	enum 
	{
		NUM_SECTORS = 32,
	};
	
	std::vector<int> sectors_;
	Vector3 pos_;
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

/*
 */
class Expression 
{
public:
	Expression(const char *str);
	Expression(const Expression &expression);
	~Expression();
	
	Matrix4 to_matrix(float time);
	
protected:
	
	char *exp_[14];
};
