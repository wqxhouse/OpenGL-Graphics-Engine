#pragma once
#include <utility>
#include "Vector4.h"

#define MATH_INF 1e+8f
#define MATH_PI  3.141592654f
#define MATH_2PI 6.283185308f
#define MATH_HPI 1.570796327f
#define MATH_EPSILON 1e-6f

#define MATH_DEG2RAD (PI / 180.0f)
#define MATH_RAD2DEG (180.0f / PI)

class Vector3;
class BasicMath
{
public:
	typedef std::pair<Vector3, Vector3> VertexMinMax;

	BasicMath(void);
	virtual ~BasicMath(void) = 0;

	static const Vector3 Vec3_INF;
	static VertexMinMax calculateMinMaxFromVertices(const Vector3 *vertexCoordArr, int arrSize);
	static float lengthSquare(const Vector3 &v);

	static Vector3 getVecMin(const Vector3 &v0, const Vector3 &v1);
	static Vector3 getVecMax(const Vector3 &v0, const Vector3 &v1);

};

//
//struct quat {
//
//	quat() : x(0), y(0), z(0), w(1) { }
//	quat(const vec3 &dir,float angle) {
//		set(dir,angle);
//	}
//	quat(float x,float y,float z,float angle) {
//		set(x,y,z,angle);
//	}
//	quat(const mat3 &m) {
//		float trace = m[0] + m[4] + m[8];
//		if(trace > 0.0) {
//			float s = sqrt(trace + 1.0f);
//			q[3] = 0.5f * s;
//			s = 0.5f / s;
//			q[0] = (m[5] - m[7]) * s;
//			q[1] = (m[6] - m[2]) * s;
//			q[2] = (m[1] - m[3]) * s;
//		} else {
//			static int next[3] = { 1, 2, 0 };
//			int i = 0;
//			if(m[4] > m[0]) i = 1;
//			if(m[8] > m[3 * i + i]) i = 2;
//			int j = next[i];
//			int k = next[j];
//			float s = sqrt(m[3 * i + i] - m[3 * j + j] - m[3 * k + k] + 1.0f);
//			q[i] = 0.5f * s;
//			if(s != 0) s = 0.5f / s;
//			q[3] = (m[3 * j + k] - m[3 * k + j]) * s;
//			q[j] = (m[3 * i + j] + m[3 * j + i]) * s;
//			q[k] = (m[3 * i + k] + m[3 * k + i]) * s;
//		}
//	}
//
//	operator float*() { return (float*)&x; }
//	operator const float*() const { return (float*)&x; }
//
//	float &operator[](int i) { return ((float*)&x)[i]; }
//	const float operator[](int i) const { return ((float*)&x)[i]; }
//
//	quat operator*(const quat &q) const {
//		quat ret;
//		ret.x = w * q.x + x * q.x + y * q.z - z * q.y;
//		ret.y = w * q.y + y * q.w + z * q.x - x * q.z;
//		ret.z = w * q.z + z * q.w + x * q.y - y * q.x;
//		ret.w = w * q.w - x * q.x - y * q.y - z * q.z;
//		return ret;
//	}
//
//	void set(const vec3 &dir,float angle) {
//		float length = dir.length();
//		if(length != 0.0) {
//			length = 1.0f / length;
//			float sinangle = sin(angle * DEG2RAD / 2.0f);
//			x = dir[0] * length * sinangle;
//			y = dir[1] * length * sinangle;
//			z = dir[2] * length * sinangle;
//			w = cos(angle * DEG2RAD / 2.0f);
//		} else {
//			x = y = z = 0.0;
//			w = 1.0;
//		}
//	}
//	void set(float x,float y,float z,float angle) {
//		set(vec3(x,y,z),angle);
//	}
//
//	void slerp(const quat &q0,const quat &q1,float t) {
//		float k0,k1,cosomega = q0.x * q1.x + q0.y * q1.y + q0.z * q1.z + q0.w * q1.w;
//		quat q;
//		if(cosomega < 0.0) {
//			cosomega = -cosomega;
//			q.x = -q1.x;
//			q.y = -q1.y;
//			q.z = -q1.z;
//			q.w = -q1.w;
//		} else {
//			q.x = q1.x;
//			q.y = q1.y;
//			q.z = q1.z;
//			q.w = q1.w;
//		}
//		if(1.0 - cosomega > 1e-6) {
//			float omega = acos(cosomega);
//			float sinomega = sin(omega);
//			k0 = sin((1.0f - t) * omega) / sinomega;
//			k1 = sin(t * omega) / sinomega;
//		} else {
//			k0 = 1.0f - t;
//			k1 = t;
//		}
//		x = q0.x * k0 + q.x * k1;
//		y = q0.y * k0 + q.y * k1;
//		z = q0.z * k0 + q.z * k1;
//		w = q0.w * k0 + q.w * k1;
//	}
//
//	mat3 to_matrix() const {
//		mat3 ret;
//		float x2 = x + x;
//		float y2 = y + y;
//		float z2 = z + z;
//		float xx = x * x2;
//		float yy = y * y2;
//		float zz = z * z2;
//		float xy = x * y2;
//		float yz = y * z2;
//		float xz = z * x2;
//		float wx = w * x2;
//		float wy = w * y2;
//		float wz = w * z2;
//		ret[0] = 1.0f - (yy + zz); ret[3] = xy - wz; ret[6] = xz + wy;
//		ret[1] = xy + wz; ret[4] = 1.0f - (xx + zz); ret[7] = yz - wx;
//		ret[2] = xz - wy; ret[5] = yz + wx; ret[8] = 1.0f - (xx + yy);
//		return ret;
//	}
//
//	union {
//		struct {
//			float x,y,z,w;
//		};
//		float q[4];
//	};

