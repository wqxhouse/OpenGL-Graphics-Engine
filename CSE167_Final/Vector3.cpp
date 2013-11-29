#include "Vector3.h"
#include <cmath>
#include <iostream>
#include "assert.h"

Vector3::Vector3(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

Vector3::Vector3():
	m_x(0.0f), 
	m_y(0.0f),
	m_z(0.0f) {}


Vector3::~Vector3()
{

}

float Vector3::angle(const Vector3 &v) const
{
	if((v['x'] == 0.0f && v['y'] == 0.0f && v['z'] == 0.0f) ||
		(m_x == 0.0f && m_y == 0.0f && m_z == 0.0f))
	{
		return -1;
	}

	//get copies to normalize
	Vector3 v1 = v;
	Vector3 v0 = *this;

	v1.normalize();
	v0.normalize();

	float angle = acosf(v0.dot(v1));
	angle = angle * 180 / 3.14159265358979;

	return angle;
}

//Element access 'set': set the vector coordinates
void Vector3::set(float value, char axis)
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
	default:
		assert(false);
	
	}
}

void Vector3::set(float x, float y, float z)
{
	m_x = x;
	m_y = y;
	m_z = z;
}

void Vector3::set(int startIndex, const std::vector<float> &vertices)
{
	std::vector<float>::const_iterator it = vertices.begin() + startIndex;
	m_x = *it++;
	m_y = *it++;
	m_z = *it;
}


//Element access 'get': return a specific coordinate of the vector
float Vector3::get(char axis) const
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
	default:
		assert(false);
		return NULL;
	}
}

//Overload operator '[]' for 'get' access
float Vector3::operator [] (char axis) const
{
	return get(axis);
}

//Vector addition
Vector3 Vector3::add(const Vector3 &v) const
{
	float x = v.m_x + m_x;
	float y = v.m_y + m_y;
	float z = v.m_z + m_z;

	return Vector3(x, y, z);
}

Vector3 Vector3::Add(const Vector3 &a, const Vector3 &b)
{
	float x = a.m_x + b.m_x;
	float y = a.m_y + b.m_y;
	float z = a.m_z + b.m_z;

	return Vector3(x, y, z);
}

	//Overload operator '+' for addition
Vector3	Vector3::operator + (const Vector3 &v) const
{
	return add(v);
}

//Vector subtraction
Vector3 Vector3::subtract(const Vector3 &v) const
{
	float x = m_x - v.m_x;
	float y = m_y - v.m_y;
	float z = m_z - v.m_z;

	return Vector3(x, y, z);
}


Vector3 Vector3::Subtract(const Vector3 &a, const Vector3 &b)
{
	float x = a.m_x - b.m_x;
	float y = a.m_y - b.m_y;
	float z = a.m_z - b.m_z;

	return Vector3(x, y, z);
}

//Overload operator '-' for subtraction
Vector3 Vector3::operator - (const Vector3 &v) const
{
	return subtract(v);
}

//Negation
Vector3 Vector3::negate() const
{
	float x = -m_x;
	float y = -m_y;
	float z = -m_z;

	return Vector3(x, y, z);
}


Vector3 Vector3::Negate(const Vector3 &v)
{
	float x = -v.m_x;
	float y = -v.m_y;
	float z = -v.m_z;

	return Vector3(x, y, z);
}

//Scale (multiplication with scalar value)

Vector3 Vector3::scale(float factor) const
{
	float x = factor * m_x;
	float y = factor * m_y;
	float z = factor * m_z;

	return Vector3(x, y, z);
}

Vector3 Vector3::Scale(float factor, const Vector3 &v)
{
	float x = factor * v.m_x;
	float y = factor * v.m_y;
	float z = factor * v.m_z;
	
	return Vector3(x, y, z);
}


//Dot product
float Vector3::dot(const Vector3 &v)
{
	return m_x * v.m_x +
		   m_y * v.m_y +
		   m_z * v.m_z;
}

//Cross product
Vector3 Vector3::cross(const Vector3 &v)
{
	return Vector3(
		(m_y*v.m_z) - (m_z*v.m_y), 
		(m_z*v.m_x) - (m_x*v.m_z), 
		(m_x*v.m_y) - (m_y*v.m_x)
		);
}

//Magnitude (length of vector)
float Vector3::getLength() const
{
	return sqrtf(m_x*m_x + m_y*m_y + m_z*m_z);
}

//Normalize
void Vector3::normalize()
{
	float length = getLength();

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

}

Vector3 Vector3::getNormalizedVec() const
{
	float length = getLength();

	if(length == 0.0f)
	{
		assert(false);
	}

	if(length == 1)
	{
		return Vector3(m_x, m_y, m_z);
	}

	// mul since div is expensive
	float factor = 1.0f/length;
	return Vector3(
		m_x * factor,
		m_y * factor,
		m_z * factor);
}

//Print (display the vector's components numerically on the screen)
void Vector3::print() const 
{
	std::cout<< "X: " <<m_x <<std::endl
			 << "Y: " <<m_y <<std::endl
			 << "Z: " <<m_z <<std::endl;
}

void Vector3::Print(Vector3 &v)
{
	std::cout<< "X: " <<v.m_x <<std::endl
			 << "Y: " <<v.m_y <<std::endl
			 << "Z: " <<v.m_z <<std::endl;
}

Vector3 Vector3::Normalize(const Vector3& v)
{
	float length = v.getLength();

	if(length == 0.0f)
	{
		assert(false);
	}

	if(length == 1)
	{
		return v;
	}

	Vector3 vTemp(v['x'], v['y'], v['z']);
	// mul since div is expensive
	float factor = 1.0f/length;
	vTemp.set(v['x'] * factor, 'x');
	vTemp.set(v['y'] * factor, 'y');
	vTemp.set(v['z'] * factor, 'z');

	return vTemp;
}

Vector3 Vector3::Cross(const Vector3& v1, const Vector3 & v2)
{
	return Vector3(
		(v1['y']*v2['z']) - (v1['z']*v2['y']), 
		(v1['z']*v2['x']) - (v1['x']*v2['z']), 
		(v1['x']*v2['y']) - (v1['y']*v2['x']));
}