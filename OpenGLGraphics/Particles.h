#pragma once
#include "Vector3.h"
#include "Vector4.h"
//
//class Particles 
//{
//public:
//
//	Particles(int num,const Vector3 &pos,float speed,float rotation,const Vector3 &force,float time,float radius,const Vector4 &color);
//	~Particles();
//
//	void update(float ifps);
//
//	int render();
//
//	void set(const Vector3 &p);
//	void setForce(const Vector3 &f);
//	void setColor(const Vector4 &c);
//
//	const Vector3 &getMin();
//	const Vector3 &getMax();
//	const Vector3 &getCenter();
//	float getRadius();
//
//	static Vector3 OFF;
//
//protected:
//
//	float rand();
//
//	int num_particles;	// number of particles
//
//	Vector3 pos;
//	float speed;		// speed
//	float rotation;		// rotation
//	Vector3 force;			// force / mass
//	float time;			// life time
//	float radius;		// radius
//	Vector4 color;			// color
//
//	Vector3 *xyz;			// positions
//	Vector3 *speeds;		// speeds
//	float *rotations;	// rotations
//	float *times;		// times
//
//	struct Vertex {
//		Vector3 xyz;		// coordinate
//		Vector4 attrib;	// attributes (texcoord + dx + dy)
//		Vector4 color;		// color
//		vec2 sincos;	// sin(rotation) + cos(rotation)
//	};
//
//	int num_vertex;
//	Vertex *vertex;
//
//	Vector3 min;			// bound box
//	Vector3 max;
//	Vector3 center;
//};