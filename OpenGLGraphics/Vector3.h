#pragma once

#include <vector>
#define MATH_EPSILON 1e-6f
class Vector3
{
public:

// Methods ////////////////////////

	//Element access 'set': set the vector coordinates
	void set(float value, char axis);
	void set(float x, float y, float z);
	void set(int startIndex, const std::vector<float> &vertices);

	//Element access 'get': return a specific coordinate of the vector
	float get(char axis) const;

	//Overload operator '[]' for 'get' access
	float operator [] (char axis) const;

	//Vector addition
	Vector3 add(const Vector3 &v) const;
	static Vector3 Add(const Vector3 &a, const Vector3 &b);

	//Overload operator '+' for addition
	Vector3 operator + (const Vector3 &v) const;

	//Vector subtraction
	Vector3 subtract(const Vector3 &v) const;
	static Vector3 Subtract(const Vector3 &a, const Vector3 &b);

	//Overload operator '-' for subtraction
	Vector3 operator - (const Vector3 &v) const;

	//Negation
	Vector3 negate() const;
	static Vector3 Negate(const Vector3 &v);

	//Scale (multiplication with scalar value)

	Vector3 scale(float factor) const;
	static Vector3 Scale(float factor, const Vector3 &v);

	//Dot product
	float dot(const Vector3 &v);

	//Cross product
	Vector3 cross(const Vector3 &v);

	//Magnitude (length of vector)
	float getLength() const;

	//Normalize
	void normalize();
	Vector3 getNormalizedVec() const;

	//Print (display the vector's components numerically on the screen)
	void print() const;
	static void Print(Vector3 &v);

	float angle(const Vector3 &v) const;

	inline int operator==(const Vector3 &v) 
	{ 
		return (std::fabs(m_x - v.m_x) < MATH_EPSILON 
			 && std::fabs(m_y - v.m_y) < MATH_EPSILON 
			 && std::fabs(m_z - v.m_z) < MATH_EPSILON); 
	}

	inline int operator!=(const Vector3 &v)
	{ 
		return !(*this == v); 
	}

// Constructors ///////////
	Vector3();
	Vector3(float x, float y, float z);
	Vector3(const Vector3 &v);
	~Vector3();

	static Vector3 Normalize(const Vector3& v);
	static Vector3 Cross(const Vector3& v1, const Vector3 & v2);

	inline const float *getPointer() const
	{
		return &m_x;
	}

private:
	float m_x, m_y, m_z;
};

