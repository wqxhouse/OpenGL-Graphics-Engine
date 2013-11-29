#pragma once
#include <utility>
#include "Vector3.h"
#include "Vector4.h"

#define MATH_INF 1e+8f
#define MATH_PI  3.141592654f
#define MATH_2PI 6.283185308f
#define MATH_HPI 1.570796327f
#define MATH_EPSILON 1e-6f

#define MATH_DEG2RAD (PI / 180.0f)
#define MATH_RAD2DEG (180.0f / PI)

class BasicMath
{
public:
	typedef std::pair<Vector3, Vector3> VertexMinMax;

	BasicMath(void);
	virtual ~BasicMath(void) = 0;

	static const Vector3 Vec3_INF;
	static VertexMinMax calculateMinMaxFromVertices(const Vector3 *vertexCoordArr, int arrSize);
	static float lengthSquare(const Vector3 &v);
};

