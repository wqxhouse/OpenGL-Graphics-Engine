#pragma once
#include "Vector3.h"

class Vector4
{
public:

	// Methods /////////////////////////

	//Element access 'set': set the (four) point coordinates
	void set(float value, char axis);
	void set(float x, float y, float z, float w);

	//Element access 'get': return one of the four point coordinates
	float get(char axis) const;

	//Overload operator '[]' for 'get' access
	float operator [] (char axis) const;

	//Vector addition
	Vector4 add(const Vector4 &v);
	static Vector4 Add(const Vector4 &a, const Vector4 &b);

	//Overload operator '+' for addition
	Vector4 operator + (const Vector4 &v);

	//Vector subtraction
	Vector4 subtract(const Vector4 &v);
	static Vector4 Subtract(const Vector4 &a, const Vector4 &b);

	//Overload operator '-' for subtraction
	Vector4 operator - (const Vector4 &v);
	inline Vector4 negate() const { return Vector4(-m_x, -m_y, -m_z, -m_w); }
	float dot(const Vector4 &v) const;
	float operator * (const Vector4 &v) const;
	float dotVec3(const Vector3 &v) const;
	Vector4 scale(float factor) const;

	Vector3 getVector3() const;
	void setVector3(const Vector3 &v);

	//Dehomogenize (make fourth component equal to 1)
	void dehomogenize();
	void normalizePlane();


	//Print (display the point's components numerically on the screen)
	void print() const;
	static void Print(Vector4 &v);
	static Vector4 Scale(const Vector4 &v, float factor);


	// Constructors ////////////////////
	Vector4();
	Vector4(const Vector3 &v, float w);

	//A constructor with three (or four, optionally) parameters for the point coordinates
	Vector4(float x, float y, float z);
	Vector4(float x, float y, float z, float w);

	inline const float *getPointer() const
	{
		return &m_x;
	}

	~Vector4();

private:
	float m_x, m_y, m_z, m_w;


};

