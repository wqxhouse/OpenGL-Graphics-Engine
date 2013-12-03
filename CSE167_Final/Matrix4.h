#pragma once
#include "Vector3.h"
#include "Vector4.h"

#define DEG_TO_RAD 0.017453292519943295769236907684886f

class Matrix4
{
public:

	// Methods ///////////////////////////////////

	//Multiply (matrix-times-matrix)
	Matrix4 multiplyMat(const Matrix4 &mat) const;

	//Multiply (matrix-times-vector): if multiplying with a point, 
	//dehomogenize the result after the multiplication
	Vector4 multiplyVec4(const Vector4 &v4) const;
	Vector3 multiplyVec3(const Vector3 &v3) const;

	//Make rotation matrix about x axis
	void setRotationX(const float angle);
	
	//Make rotation matrix about y axis
	void setRotationY(const float angle);
	
	//Make rotation matrix about z axis
	void setRotationZ(const float angle);
	
	void setRotationLine(const Vector3 &vec, const Vector3& point, const float angle);

	//Make rotation matrix about arbitrary (unit) axis
	void setRotationVec(const Vector3 &axis, const float angle);

	
	//Make non-uniform scaling matrix
	void setScale(Vector3 &factor);
	
	//Make translation matrix
	void setTranslate(const Vector3 &factor);
	
	//Print (display all 16 matrix components numerically on the screen in a 4x4 array)
	void print() const;

	//get the matrix entry by index
	float operator [] (int index) const;

	//set the matrix entries
	void set(int index, float value);

	//load the identity matrix
	void loadIdentity();

	void getMatrixArray(float m[]) const;

	Matrix4 getTranspose();

	void ResetTranslate();

	void setFrustumMat(float l, float r, float b, float t, float n, float f);
	void setLookAtMat(const Vector3 &eye,const Vector3 &dir,const Vector3 &up);
	void setPerspectiveMat(float fov,float aspect,float znear,float zfar);
	void setViewportMat(float x0, float y0, float x1, float y1);

	float getDet() const;
	Matrix4 getInverse() const;
	Matrix4 getRotationPart() const;
	inline const float* getPointer() const
	{
		return &m_entries[0];
	}

	// Static Matrix Creators ////////////////////
	static Matrix4 MakeTranslate(const Vector3 &trans);
	static Matrix4 MakeRotationX(const float angle);
	static Matrix4 MakeRotationY(const float angle);
	static Matrix4 MakeRotationZ(const float angle);
	static Matrix4 MakeRotationVec(const Vector3 &axis, const float angle);
	static Matrix4 MakeUniScale(const float factor);

	// Constructors //////////////////////////////
	Matrix4();
	Matrix4(const float *m);

	//Row Major Matrix - conforms to DirectX standard
	Matrix4(
		float m11, float m12, float m13, float m14,
		float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34,
		float m41, float m42, float m43, float m44);
	~Matrix4();

private:
	friend class Quat; //ugly consider refactor
	float m_entries[16];

};

