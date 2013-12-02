#include "Light.h"
#include "core.h"

Light::Light(const Vector3 &pos,float radius,const Vector4 &color,int shadows) 
	: color_(color), 
	  shadows_(shadows), 
	  material_(nullptr), 
	  time_(0.0), 
	  pos_(pos)
{
	pos_.setRadius(radius);
}

Light::~Light() 
{
}

/*
 */
void Light::update(float spf) 
{
	time_ += spf;
	pos_.update(time_, transform_);
}

/*
 */
int Light::bindMaterial(const char *name,Material *material) 
{
	material_ = material;
	return 1;
}

/*
 */
void Light::setPosition(const Vector3 &p) 
{
	transform_.setTranslate(p);
	pos_.setPosition(p);
}

/*
 */
void Light::setTransform(const Matrix4 &m) 
{
	transform_ = m;
	Vector3 transformedpos = m.multiplyVec3(Vector3(0,0,0));
	pos_.setPosition(transformedpos);
}

/*
 */
void Light::setColor(const Vector4 &c) {
	color_ = c;
}

/*
 */
void Light::getScissor(int *scissor) 
{
	if((pos_.getPosCoord() - Core::camera_.getPosCoord()).getLength() < pos_.bsphere_.getRadius() * 1.5) 
	{
		scissor[0] = Core::viewport_[0];
		scissor[1] = Core::viewport_[1];
		scissor[2] = Core::viewport_[2];
		scissor[3] = Core::viewport_[3];
		return;
	}

	Matrix4 tmodelview = Core::modelview_.getTranspose();
	Matrix4 mvp = Core::projection_.multiplyMat(Core::modelview_);
	Vector3 x = tmodelview.multiplyVec3(Vector3(radius(), 0, 0));
	Vector3 y = tmodelview.multiplyVec3(Vector3(0, radius(), 0));

	Vector4 p[4];
	p[0] = mvp.multiplyVec4(Vector4(pos() - x, 1));
	p[1] = mvp.multiplyVec4(Vector4(pos() + x, 1));
	p[2] = mvp.multiplyVec4(Vector4(pos() - y, 1));
	p[3] = mvp.multiplyVec4(Vector4(pos() + y, 1));
	p[0] = Vector4::Scale(p[0], 1 / p[0]['w']);
	p[1] = Vector4::Scale(p[1], 1 / p[1]['w']);
	p[2] = Vector4::Scale(p[2], 1 / p[2]['w']);
	p[3] = Vector4::Scale(p[3], 1 / p[3]['w']);

	if(p[0]['x'] < p[2]['x']) 
	{
		scissor[0] = Core::viewport_[0] + (int)((float)Core::viewport_[2] * (p[0]['x'] + 1.0) / 2.0);
		scissor[2] = Core::viewport_[0] + (int)((float)Core::viewport_[2] * (p[1]['x'] + 1.0) / 2.0);
	} 
	else 
	{
		scissor[0] = Core::viewport_[0] + (int)((float)Core::viewport_[2] * (p[1]['x'] + 1.0) / 2.0);
		scissor[2] = Core::viewport_[0] + (int)((float)Core::viewport_[2] * (p[0]['x'] + 1.0) / 2.0);
	}
	if(p[1]['y'] < p[3]['y']) 
	{
		scissor[1] = Core::viewport_[1] + (int)((float)Core::viewport_[3] * (p[2]['y'] + 1.0) / 2.0);
		scissor[3] = Core::viewport_[1] + (int)((float)Core::viewport_[3] * (p[3]['y'] + 1.0) / 2.0);
	} 
	else 
	{
		scissor[1] = Core::viewport_[1] + (int)((float)Core::viewport_[3] * (p[3]['y'] + 1.0) / 2.0);
		scissor[3] = Core::viewport_[1] + (int)((float)Core::viewport_[3] * (p[2]['y'] + 1.0) / 2.0);
	}
	if(scissor[0] < Core::viewport_[0])
	{
		scissor[0] = Core::viewport_[0];
	}
	else if(scissor[0] > Core::viewport_[0] + Core::viewport_[2])
	{
		scissor[0] = Core::viewport_[0] + Core::viewport_[2];
	}
	if(scissor[1] < Core::viewport_[1])
	{
		scissor[1] = Core::viewport_[1];
	}
	else if(scissor[1] > Core::viewport_[1] + Core::viewport_[3])
	{
		scissor[1] = Core::viewport_[1] + Core::viewport_[3];
	}
	if(scissor[2] < Core::viewport_[0])
	{
		scissor[2] = Core::viewport_[0];
	}
	else if(scissor[2] > Core::viewport_[2] + Core::viewport_[3])
	{
		scissor[2] = Core::viewport_[0] + Core::viewport_[2];
	}
	if(scissor[3] < Core::viewport_[1])
	{
		scissor[3] = Core::viewport_[1];
	}
	else if(scissor[3] > Core::viewport_[1] + Core::viewport_[3])
	{
		scissor[3] = Core::viewport_[1] + Core::viewport_[3];
	}
	scissor[2] -= scissor[0];
	scissor[3] -= scissor[1];
}
