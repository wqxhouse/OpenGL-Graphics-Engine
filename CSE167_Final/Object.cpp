#include "Object.h"

Object::Object(int type)
	: type_(type), 
	  is_identity_(true), 
	  shadows_(true), 
	  rigidbody_(nullptr), 
	  time_(0), 
	  frame_(0), 
	  pos_(Vector3(0, 0, 0))
{
}


Object::~Object()
{
	for(int i = 0; i < pos_.sectors_.size(); i++) 
	{
		BSPTree::sectors_[pos_.sectors_[i]].removeObject(this);
	}

	if(rigidbody_) 
	{
		delete rigidbody_;
	}
}

void Object::update(float spf)
{
	time_ += spf;
	if(rigidbody_) 
	{
		/*if(Core::physics_toggle)
		{
		rigidbody->simulate();
		}*/
	} 
	else 
	{
		pos_.update(time_, this);
	}
}	

void Object::updatePos(const Vector3 &p)
{
	for(int i = 0; i < pos_.sectors_.size(); i++)
	{
		BSPTree::sectors_[pos_.sectors_.at(i)].removeObject(this);
	}

	pos_.bsphere_.set(p, getRadius());

	for(int i = 0; i < pos_.sectors_.size(); i++) 
	{
		BSPTree::sectors_[pos_.sectors_.at(i)].addObject(this);
	}
}

int Object::bindMaterial(const char *name, Material *material)
{
	//TODO: after integrating the xml system
	return 0;
}

void Object::setRigidBody(RigidBody *rigidbody)
{
	is_identity_ = 0;
	rigidbody_ = rigidbody;
}

void Object::set_position(const Vector3 &p)
{
	is_identity_ = 0;
	transform_.setTranslate(p);
	itransform_ = transform_.getInverse();
	updatePos(p);
	/*if(rigidbody_) 
	{
	rigidbody_->set(p);
	}*/
}

void Object::set_transform(const Matrix4 &m)
{
	is_identity_ = 0;
	transform_ = m;
	itransform_ = transform_.getInverse();
	updatePos(m.multiplyVec3(Vector3(0,0,0)));
	//TODO: reapply shader
	/*if(rigidbody_)
	{
	rigidbody_->set(m);
	}*/
}

void Object::enable()
{
	if(is_identity_)
	{
		return;
	}

	prev_modelview_ = Core::modelview_;
	prev_imodelview_ = Core::imodelview_;
	prev_transform_ = Core::transform_;
	prev_itransform_ = Core::itransform_;
	Core::modelview_ = Core::modelview_.multiplyMat(transform_);
	Core::imodelview_ = itransform_.multiplyMat(Core::imodelview_);
	Core::transform_ = Core::transform_.multiplyMat(transform_);
	Core::itransform_ = itransform_.multiplyMat(Core::itransform_);

	float matPOD[16];
	Core::modelview_.getMatrixArray(matPOD);
	glLoadMatrixf(matPOD);
	//TODO: bind shader

	// new transformation
	/*if(Shader::old_shader)
	{
	Shader::old_shader->bind();
	}*/
}

void Object::disable()
{
	if(is_identity_)
	{
		return;
	}
	Core::modelview_ = prev_modelview_;
	Core::imodelview_ = prev_imodelview_;
	Core::transform_ = prev_transform_;
	Core::itransform_ = prev_itransform_;

	float matPOD[16];
	Core::modelview_.getMatrixArray(matPOD);

	glLoadMatrixf(matPOD);
	//TODO:
	//if(Shader::old_shader)
	//{
	//	Shader::old_shader->bind();
	//}
}

void Object::setShadows(bool b_shadows)
{
	shadows_ = b_shadows;
}
