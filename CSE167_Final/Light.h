#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4.h"
#include "Position.h"
#include "Material.h"
#include "Bounds.h"

class Material;
class Light
{
public:

	Light(const Vector3 &pos,float radius,const Vector4 &color,int shadows = 1);
	~Light();

	void update(float spf);

	int bindMaterial(const char *name, Material *material);

	void renderFlare();

	void setPosition(const Vector3 &p);
	void setTransform(const Matrix4 &m);
	void setColor(const Vector4 &c);

	void getScissor(int *scissor);

	inline float radius() const
	{
		return pos_.getBoundingRadius();;
	}

	inline const Vector3& pos() const
	{
		return pos_.getPosCoord();
	}
	
	Position pos_;
	Matrix4 transform_;

	Vector4 color_;

	int shadows_;

	Material *material_;

	float time_;
};

