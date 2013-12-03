#pragma once
#include <cmath>
#include "BasicMath.h"
#include "Matrix4.h"
#include "Vector3.h"
#include "Vector4.h"
class Quat
{
public:
	Quat() : x(0), y(0), z(0), w(1) { }
	Quat(const Vector3 &dir,float angle) 
	{
		set(dir,angle);
	}
	Quat(float x,float y,float z,float angle) 
	{
		set(x,y,z,angle);
	}
	~Quat() {}
	

	Quat operator*(const Quat &q) const 
	{
		Quat ret;
		ret.x = w * q.x + x * q.x + y * q.z - z * q.y;
		ret.y = w * q.y + y * q.w + z * q.x - x * q.z;
		ret.z = w * q.z + z * q.w + x * q.y - y * q.x;
		ret.w = w * q.w - x * q.x - y * q.y - z * q.z;
		return ret;
	}

	void set(const Vector3 &dir,float angle) 
	{
		float length = dir.getLength();
		if(length != 0.0) 
		{
			length = 1.0f / length;
			float sinangle = sin(angle * MATH_DEG2RAD / 2.0f);
			x = dir['x'] * length * sinangle;
			y = dir['y'] * length * sinangle;
			z = dir['z'] * length * sinangle;
			w = cos(angle * MATH_DEG2RAD / 2.0f);
		} else {
			x = y = z = 0.0;
			w = 1.0;
		}
	}
	void set(float x,float y,float z,float angle) 
	{
		set(Vector3(x,y,z),angle);
	}
	 
	Matrix4 to_matrix() const
	{
		Matrix4 ret;
		float x2 = x + x;
		float y2 = y + y;
		float z2 = z + z;
		float xx = x * x2;
		float yy = y * y2;
		float zz = z * z2;
		float xy = x * y2;
		float yz = y * z2;
		float xz = z * x2;
		float wx = w * x2;
		float wy = w * y2;
		float wz = w * z2;
		ret.m_entries[0] = 1.0f - (yy + zz); ret.m_entries[4] = xy - wz;			ret.m_entries[8] = xz + wy;			ret.m_entries[12] = 0;
		ret.m_entries[1] = xy + wz;          ret.m_entries[5] = 1.0f - (xx + zz); ret.m_entries[9] = yz - wx;			ret.m_entries[13] = 0;
		ret.m_entries[2] = xz - wy;          ret.m_entries[6] = yz + wx;			ret.m_entries[10] = 1.0f - (xx + yy); ret.m_entries[14] = 0;
		ret.m_entries[3] = 0;                ret.m_entries[7] = 0;				ret.m_entries[11] = 0;				ret.m_entries[15] = 1;
		return ret;
	}

	union {
		struct {
			float x,y,z,w;
		};
		float q[4];
	};

};

