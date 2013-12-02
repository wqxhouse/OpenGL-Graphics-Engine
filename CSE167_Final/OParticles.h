#pragma once
#include "Object.h"
#include "Particles.h"

class OParticles
	: public Object
{
public:
	explicit OParticles(Particles *particle);
	virtual ~OParticles(void);

	virtual int render(int surface_id = -1);
	virtual int intersection(const Vector3 &l0,
		const Vector3 &l1, 
		Vector3 *point,
		Vector3 *normal, 
		int surface_id = -1) ;

	virtual int getNumSurfaces() ;
	virtual const std::string getSurfaceName(int surface_id) ;
	virtual int getSurface(const char *name) ;

	virtual const Vector3 &getMin(int surface_id = -1) ;
	virtual const Vector3 &getMax(int surface_id = -1) ;

	virtual const Vector3 &getCenter(int surface_id = -1) ;
	virtual float getRadius(int surface_id = -1) ;

	Particles particle_;
	float off_time_;

};

