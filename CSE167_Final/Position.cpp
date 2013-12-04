#include "Position.h"
#include "BSPTree.h"
#include "Object.h"

Position::Position()
   : spline_(nullptr), 
   in_sector_id_(-1),
   bsphere_(), 
   sectors_()
{

}

Position::Position(const Vector3 &pos) 
	: spline_(nullptr), 
	  expression_(nullptr), 
	  in_sector_id_(-1), 
	  bsphere_(pos, 0.0)
{
	//sectors_.resize(NUM_SECTORS);
}

Position::~Position() 
{
	if(spline_) delete spline_;
	sectors_.clear();
}

void Position::find(int sector,float r) 
{
	if(sectors_.size() == NUM_SECTORS) 
	{
		fprintf(stderr,"Position::find(): this object presents in %d sectors\n", sectors_.size());
		return;
	}

	sectors_.push_back(sector);

	if(bsphere_.getRadius() < 0.0) return;

	Sector *s = &BSPTree::sectors_[sector];
	for(int i = 0; i < s->portals_.size(); i++) 
	{
		Portal *p = &BSPTree::portals_[s->portals_[i]];
		if((getPosCoord() - p->bsphere_.getCenter().getVector3()).getLength() 
	        > r + p->bsphere_.getRadius())
		{
			continue;
		}
		for(int j = 0; j < p->sectors_.size(); j++) 
		{
			int k = 0;
			for(; k < sectors_.size(); k++)
			{
				if(sectors_[k] == p->sectors_[j]) break;
			}
			if(k != sectors_.size())
			{
				continue;
			}
			if(BSPTree::sectors_[p->sectors_[j]].inside(bsphere_))
			{
				find(p->sectors_[j],r - (getPosCoord() - p->bsphere_.getCenter().getVector3()).getLength());
			}
		}
	}
}

/*
 */
void Position::setSpline(Spline *spline)
{
	spline_ = spline;
	expression_ = nullptr;
}

void Position::setExpression(Expression *expression)
{
	spline_ = nullptr;
	expression_ = expression;
}

/*
 */
void Position::setRadius(float radius) 
{
	bsphere_.setRadius(radius);
}


/*
 */
void Position::update(float time,Matrix4 &transform)
{
	if(spline_) 
	{
		transform = spline_->to_matrix(time);
		operator=(transform.multiplyVec3(Vector3(0,0,0)));
	} 
}

/*
 */
void Position::update(float time,Object *object) 
{
	if(spline_)
	{
		if(object) 
		{
			object->is_identity_ = 0;
			for(int i = 0; i < sectors_.size(); i++)
			{
				BSPTree::sectors_[sectors_[i]].removeObject(object);
			}
			bsphere_.setRadius(object->getRadius());
		}

		Matrix4 transform;
		if(spline_) transform = spline_->to_matrix(time);
		operator=(transform.multiplyVec3(Vector3(0,0,0)));
		if(object) 
		{
			object->transform_ = transform;
			object->itransform_ = transform.getInverse();
			for(int i = 0; i < sectors_.size(); i++)
			{
				BSPTree::sectors_[sectors_[i]].addObject(object);
			}
		}
	}
}

/*
 */
Matrix4 Position::to_matrix(float time) 
{
	if(spline_) return spline_->to_matrix(time);
	Matrix4 transform;
	transform.setTranslate(getPosCoord());
	return transform;
}

void Position::copyPosition(const Position &pos)
{
	bsphere_.set(pos.getPosCoord(), pos.getBoundingRadius());
	spline_ = pos.spline_ ? new Spline(*pos.spline_) : nullptr;
	in_sector_id_ = pos.in_sector_id_;

	sectors_.resize(pos.sectors_.size());

	for(int i = 0; i < sectors_.size(); i++)
	{
		sectors_[i] = pos.sectors_[i];
	}
}

void Position::setPosition(const Vector3 &v)
{
	if(BSPTree::sectors_.size() == 0)
	{
		return;
	}

	bsphere_.setCenter(v);

	sectors_.resize(0);

	if(in_sector_id_ == -1) {	// find in all sectors
		for(int i = 0; i < BSPTree::sectors_.size(); i++) 
		{
			if(BSPTree::sectors_[i].inside(Vector3(getPosCoord()))) 
			{
				in_sector_id_ = i;
				find(in_sector_id_, getBoundingRadius());
				return;
			}
		}
	} 
	else 
	{
		if(BSPTree::sectors_[in_sector_id_].inside(getPosCoord()) == 0) 
		{
			Sector *s = &BSPTree::sectors_[in_sector_id_];
			for(int i = 0; i < s->portals_.size(); i++) 
			{	// find in neighborning sectors
				Portal *p = &BSPTree::portals_[s->portals_[i]];
				for(int j = 0; j < p->sectors_.size(); j++) 
				{
					if(p->sectors_[j] == in_sector_id_)
					{
						continue;
					}
					if(BSPTree::sectors_[p->sectors_[j]].inside(getPosCoord())) 
					{
						in_sector_id_ = p->sectors_[j];
						find(in_sector_id_, getBoundingRadius());
						return;
					}
				}
			}
			for(int i = 0; i < BSPTree::sectors_.size(); i++) 
			{	// find in all sectors
				if(BSPTree::sectors_[i].inside(getPosCoord())) 
				{
					in_sector_id_ = i;
					find(in_sector_id_, getBoundingRadius());
					return;
				}
			}
			in_sector_id_ = -1;
		}
	}
	if(in_sector_id_ != -1)
	{
		find(in_sector_id_, getBoundingRadius());
	}

}


Position &Position::operator=(const Position &pos) 
{
	copyPosition(pos);
	return *this;
}

Position &Position::operator=(const Vector3 &pos) 
{
	if(BSPTree::sectors_.size() == 0)
	{
		return *this;
	}
	setPosition(pos);
	return *this;
}


Spline::Spline(const char *name,float speed,int close,int follow) 
	: num_(0), 
	  params_(NULL), 
	  speed_(speed), 
	  length_(0.0), 
	  follow_(follow) 
{
	FILE *file = fopen(name,"r");
	if(!file) 
	{
		fprintf(stderr,"Spline::Spline(): error open \"%s\" file\n",name);
		return;
	}
	
	Vector3 v;
	float xx, yy, zz;
	while(fscanf(file,"%f %f %f",&xx,&yy,&zz) == 3)
	{
		num_++;
	}
	Vector3 *val = new Vector3[num_];
	
	num_ = 0;
	fseek(file,0,SEEK_SET);
	while(fscanf(file,"%f %f %f",&xx,&yy,&zz) == 3)
	{
		v.set(xx, 'x'); v.set(yy, 'y'); v.set(zz, 'z');
		val[num_++] = v;
	}
	fclose(file);
	
	float tension = 0.0;
	float bias = 0.0;
	float continuity = 0.0;
	
	params_ = new Vector3[num_ * 4];
	for(int i = 0; i < num_; i++) 
	{
		Vector3 prev,cur,next;
		if(i == 0) 
		{
			if(close) prev = val[num_ - 1];
			else prev = val[i];
			cur = val[i];
			next = val[i + 1];
		} 
		else if(i == num_ - 1) 
		{
			prev = val[i - 1];
			cur = val[i];
			if(close) next = val[0];
			else next = val[i];
		} 
		else 
		{
			prev = val[i - 1];
			cur  = val[i];
			next = val[i + 1];
		}

		Vector3 p0 = (cur - prev).scale(1.0f + bias);
		Vector3 p1 = (next - cur).scale(1.0f - bias);
		Vector3 r0 = (p0 + (p1 - p0).scale(0.5f * (1.0f + continuity)) ).scale(1.0f - tension);
		Vector3 r1 = (p0 + (p1 - p0).scale(0.5f * (1.0f - continuity)) ).scale(1.0f - tension);

		params_[i * 4 + 0] = cur;
		params_[i * 4 + 1] = next;
		params_[i * 4 + 2] = r0;
		if(i) params_[i * 4 - 1] = r1;
		else params_[(num_ - 1) * 4 + 3] = r1;
		length_ += (next - cur).getLength();
	}
	for(int i = 0; i < num_; i++) 
	{
		Vector3 p0 = params_[i * 4 + 0];
		Vector3 p1 = params_[i * 4 + 1];
		Vector3 r0 = params_[i * 4 + 2];
		Vector3 r1 = params_[i * 4 + 3];

		params_[i * 4 + 0] = p0;
		params_[i * 4 + 1] = r0;
		params_[i * 4 + 2] = p0.negate().scale(3.0f) + p1.scale(3.0f) - r0.scale(2.0f) - r1;
		params_[i * 4 + 3] = p0.scale(2.0f) - p1.scale(2.0f) + r0 + r1;
	}
	
	delete val;
}

Spline::Spline(const Spline &spline) 
{
	num_ = spline.num_;
	params_ = new Vector3[num_ * 4];
	memcpy(params_,spline.params_,sizeof(Vector3) * num_ * 4);
	speed_ = spline.speed_;
	length_ = spline.length_;
	follow_ = spline.follow_;
}

Spline::~Spline() 
{
	if(params_) delete params_;
}

/*
 */
Matrix4 Spline::to_matrix(float time) 
{
	if(!params_) return Matrix4();
	time *= speed_ / length_ * (float)num_;
	int i = (int)time;
	time -= i;
	i = (i % num_) * 4;
	float time2 = time * time;
	float time3 = time2 * time;
	Vector3 pos = params_[i + 0] 
	            + params_[i + 1].scale(time) 
				+ params_[i + 2].scale(time2) 
				+ params_[i + 3].scale(time3);

	Matrix4 transform;
	transform.setTranslate(pos);
	return transform;
}
