#include "Vector4.h"
#include <iostream>
#include "assert.h"
#include <cmath>

// Constructors ////////////////////
Vector4::Vector4():
	m_x(0.0f), 
	m_y(0.0f),
	m_z(0.0f), 
	m_w(1.0f) {}

//A constructor with three (or four, optionally) parameters for the point coordinates
Vector4::Vector4(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = 1.0f; //by default
}

Vector4::Vector4(float x, float y, float z, float w)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = w;
}

Vector4::~Vector4()
{

}

// Methods ///////////////////

//Element access 'set': set the (four) point coordinates
void Vector4::set(float value, char axis)
{
	switch(axis)
	{
	case 'x':
		m_x = value;
		break;
	case 'y':
		m_y = value;
		break;
	case 'z':
		m_z = value;
		break;
	case 'w':
		m_w = value;
		break;

	default:
		assert(false);
		
	}
}


void Vector4::set(float x, float y, float z, float w)
{
	m_x = x;
	m_y = y;
	m_z = z;
	m_w = w;
}

//Element access 'get': return one of the four point coordinates
float Vector4::get(char axis) const
{
	switch(axis)
	{
	case 'x':
		return m_x;
		break;
	case 'y':
		return m_y;
		break;
	case 'z':
		return m_z;
		break;
	case 'w':
		return m_w;
		break;
	default:
		assert(false);
		return NULL;
	}
}

//Overload operator '[]' for 'get' access
float Vector4::operator [] (char axis) const
{
	return get(axis);
}

//Vector addition
Vector4 Vector4::add(const Vector4 &v)
{
	float x = v.m_x + m_x;
	float y = v.m_y + m_y;
	float z = v.m_z + m_z;
	float w = v.m_w + m_w;

	return Vector4(x, y, z, w);
}

Vector4 Vector4::Add(const Vector4 &a, const Vector4 &b)
{
	float x = a.m_x + b.m_x;
	float y = a.m_y + b.m_y;
	float z = a.m_z + b.m_z;
	float w = a.m_w + b.m_w;

	return Vector4(x, y, z, w);
}

//Overload operator '+' for addition
Vector4 Vector4::operator + (const Vector4 &v)
{
	return add(v);
}

//Vector subtraction
Vector4 Vector4::subtract(const Vector4 &v)
{
	float x = m_x - v.m_x;
	float y = m_y - v.m_y;
	float z = m_z - v.m_z;
	float w = m_w - v.m_w;

	return Vector4(x, y, z, w);
}


Vector4 Vector4::Subtract(const Vector4 &a, const Vector4 &b)
{
	float x = a.m_x - b.m_x;
	float y = a.m_y - b.m_y;
	float z = a.m_z - b.m_z;
	float w = a.m_w - b.m_w;

	return Vector4(x, y, z, w);
}

//Overload operator '-' for subtraction
Vector4 Vector4::operator - (const Vector4 &v)
{
	return subtract(v);
}

//Dehomogenize (make fourth component equal to 1)
void Vector4::dehomogenize()
{
	m_x /= m_w;
	m_y /= m_w;
	m_z /= m_w;
	m_w = 1.0f;
}

//Print (display the point's components numerically on the screen)
void Vector4::print() const
{
	std::cout<<"X: " << m_x <<std::endl
			 <<"Y: " << m_y <<std::endl
			 <<"Z: " << m_z <<std::endl
			 <<"W: " << m_w <<std::endl;
			
}

void Vector4::Print(Vector4 &v)
{
	std::cout<<"X: " << v.m_x <<std::endl
			 <<"Y: " << v.m_y <<std::endl
			 <<"Z: " << v.m_z <<std::endl
			 <<"W: " << v.m_w <<std::endl;
}

void Vector4::normalizePlane()
{
	float length = sqrtf(m_x*m_x + m_y*m_y + m_z*m_z);

	if(length == 0.0f)
	{
		assert(false);
	}

	if(length == 1)
	{
		return;
	}

	// mul since div is expensive
	float factor = 1.0f/length;
	m_x *= factor;
	m_y *= factor;
	m_z *= factor;
	m_w *= factor;
}


float Vector4::dot(const Vector4 v) const
{
	return m_x * v.m_x + m_y * v.m_y + m_z * v.m_z + m_w * v.m_w;
}

float Vector4::operator * (const Vector4 v) const
{
	return dot(v);
}

Vector3 Vector4::getVector3() const
{
	return Vector3(m_x, m_y, m_z);
}

void Vector4::setVector3(const Vector3 &v)
{
	m_x = v.get('x');
	m_y = v.get('y');
	m_z = v.get('z');
}

Vector4 Vector4::scale(const Vector4 &v) const
{
	float x = factor * v.m_x;
	float y = factor * v.m_y;
	float z = factor * v.m_z;
	float w = factor * v.m_w;

	return Vector4(x, y, z, w);
}