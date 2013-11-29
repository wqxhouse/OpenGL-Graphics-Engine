#include "BasicMath.h"
#include <algorithm>

const Vector3 BasicMath::Vec3_INF(MATH_INF, MATH_INF, MATH_INF);

BasicMath::BasicMath(void)
{
}


BasicMath::~BasicMath(void)
{
}

BasicMath::VertexMinMax 
BasicMath::calculateMinMaxFromVertices(const Vector3 *vertexCoordArr, int arrSize)
{
	Vector3 min = Vec3_INF;
	Vector3 max = Vec3_INF.negate();

	for(int i = arrSize - 1; i >= 0; i--) 
	{
		const Vector3 &v = vertexCoordArr[i];
		if(min['x'] > v['x']) min.set(v['x'], 'x');
		if(max['x'] < v['x']) max.set(v['x'], 'x');
		if(min['y'] > v['y']) min.set(v['y'], 'y');
		if(max['y'] < v['y']) max.set(v['y'], 'y');
		if(min['z'] > v['z']) min.set(v['z'], 'z');
		if(max['z'] < v['z']) max.set(v['z'], 'z');
	}

	return std::make_pair(min, max);
}

float BasicMath::lengthSquare(const Vector3 &v)
{
	return v['x'] * v['x'] + v['y'] * v['y'] + v['z'] * v['z'];
}
