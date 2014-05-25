#include "core.h"
#include "object.h"
#include "OGeometry.h"
#include "mesh.h"
#include "rigidbody.h"
#include "Collision.h"


int Collision::counter;
Position Collision::position;
int Collision::num_surfaces;
Collision::Surface *Collision::surfaces;

/*
 */
Collision::Collision() : num_contacts(0), num_objects(0) 
{
	
	contacts = new Contact[NUM_CONTACTS];
	objects = new Object*[NUM_OBJECTS];
	
	if(counter++ == 0) {
		surfaces = new Surface[NUM_SURFACES];
		for(int i = 0; i < NUM_SURFACES; i++) {
			surfaces[i].triangles = new Triangle[NUM_TRIANGLES];
		}
	}
}

Collision::~Collision() {
	
	delete contacts;
	delete objects;
	
	if(--counter == 0) {
		for(int i = 0; i < NUM_SURFACES; i++) {
			delete surfaces[i].triangles;
		}
		delete surfaces;
	}
}

/*****************************************************************************/
/*                                                                           */
/* add contact                                                               */
/*                                                                           */
/*****************************************************************************/

int Collision::addContact(Object *object,Material *material,const Vector3 &point,const Vector3 &normal,float depth,int min_depth) {
	Contact *c = &contacts[num_contacts++];
	if(min_depth) {
		Vector3 p = object->is_identity_ ? point : object->transform_.multiplyVec3(point);
		for(int i = 0; i < num_contacts; i++) {
			if(contacts[i].point == p) 
			{
				num_contacts--;
				if(contacts[i].depth < depth) return 1;
				c = &contacts[i];
				break;
			}
		}
	}
	if(num_contacts == NUM_CONTACTS) 
	{
		num_contacts--;
		return 0;
	}
	if(num_objects == 0) 
	{
		objects[num_objects++] = object;
	} 
	else 
	{
		for(int i = 0; i < num_objects; i++) 
		{
			if(objects[i] == object) break;
			else if(i == num_objects - 1) objects[num_objects++] = object;
		}
	}
	c->object = object;
	c->material = material;
	c->point = object->is_identity_ ? point : object->transform_.multiplyVec3(point);
	c->normal = object->is_identity_ ? normal : object->transform_.getRotationPart().multiplyVec3( normal);
	c->depth = depth;
	
	return 1;
}

/*****************************************************************************/
/*                                                                           */
/* collide with sphere                                                       */
/*                                                                           */
/*****************************************************************************/

int Collision::collide(Object *object,const Vector3 &pos,float radius) 
{
	position = pos;
	return collide(object,position,radius);
}

int Collision::collide(Object *object,const Position &pos,float radius) {
	num_contacts = 0;
	num_objects = 0;
	
	if(BSPTree::sectors_.size() == 0) return 0;
	if(pos.in_sector_id_ == -1) return 0;
	
	for(int i = 0; i < pos.sectors_.size(); i++) 
	{
		Sector *s = &BSPTree::sectors_[pos.sectors_[i]];
		for(int j = 0; j < s->node_objects_.size(); j++) 
		{	// static objects
			Object *o = s->node_objects_[j];
			if((o->getCenter() - pos.getPosCoord()).getLength() >= o->getRadius() + radius) continue;
			collideObjectSphere(o,pos.getPosCoord(),radius);
		}
		for(int j = 0; j < s->objects_.size(); j++) {	// dynamic objects
			Object *o = s->objects_[j];
			if(object == o) continue;
			Vector3 p = o->is_identity_ ? pos.getPosCoord() : o->itransform_.multiplyVec3(pos.getPosCoord());
			if((o->getCenter() - p).getLength() >= o->getRadius() + radius) continue;
			/*if(object && object->rigidbody_ && object->rigidbody_->num_joints > 0 && o->rigidbody_) {
				RigidBody *rb = object->rigidbody_;
				int k;
				for(k = 0; k < rb->num_joints; k++) {
					if(rb->joined_rigidbodies[k] == o->rigidbody_) break;
				}
				if(k == rb->num_joints) collideObjectSphere(o,p,radius);
			} */
			else {
				collideObjectSphere(o,p,radius);
			}
		}
	}
	return num_contacts;
}

/*
 */
void Collision::collideObjectSphere(Object *object,const Vector3 &pos,float radius) 
{
	static int next[3] = { 1, 2, 0 };
	
	// collide sphere-Mesh
	if(object->type_ == Object::OBJ_GEOMETRY) 
	{
		// collide sphere-sphere
		/*if(object->rigidbody_ && object->rigidbody_->collide_type == RigidBody::BODY_SPHERE) 
		{
			float r = object->getRadius();
			if(pos.getLength() < radius + r) 
			{
				Vector3 normal = pos;
				normal.normalize();
				if(!addContact(object,NULL,normal.scale(radius), normal,radius + r - pos.getLength())) return;
			}
			return;
		}*/
		
		// collide sphere-Mesh
		Mesh *mesh = reinterpret_cast<OGeometry*>(object)->mesh_;
		
		for(int i = 0; i < mesh->getNumSurfaces(); i++) 
		{
			if((mesh->getCenter(i) - pos).getLength() > mesh->getRadius(i) + radius) continue;
			
			Mesh::Triangle *triangles = mesh->getTriangles(i);
			
			int num_triangles = mesh->getNumTriangles(i);
			for(int j = 0; j < num_triangles; j++) {
				Mesh::Triangle *t = &triangles[j];
				
				// distance from the center of sphere to the plane of triangle
				float dist = Vector4(pos,1) * t->plane;
				if(dist >= radius || dist < 0) continue;
				
				Vector3 normal = t->plane.getVector3();
				Vector3 point = pos - normal.scale(radius);
				float depth = radius - dist;
				
				int k;	// point in traingle
				for(k = 0; k < 3; k++) if(Vector4(point,1) * t->c[k] < 0.0) break;
				
				// collide sphere with edges
				if(k != 3) 
				{
					point = pos - normal.scale(dist);	// point on triangle plane
					for(k = 0; k < 3; k++) 
					{
						Vector3 edge = t->v[next[k]] - t->v[k];
						Vector3 dir = Vector3::Cross(edge,normal);
						dir.normalize();
						
						float d = (point - t->v[k]).dot(dir);
						if(d >= radius || d <= 0) continue;
						
						Vector3 p = point - dir.scale(d);	// point on edge
						
						float dot = p.dot(edge);	// clamp point
						if(dot > t->v[next[k]].dot(edge)) p = t->v[next[k]];
						else if(dot < t->v[k].dot(edge)) p = t->v[k];
						
						d = (point - p).getLength();
						if(d > radius) continue;
						
						depth = sqrt(radius * radius - d * d) - dist;
						if(depth <= 0.0) continue;
						
						point = p - normal.scale(depth);
						
						break;	// ok
					}
					if(k == 3) continue;	// next triangle
				}
				
				// add new contact
				if(!addContact(object,object->materials_[i],point, t->plane.getVector3(),depth)) return;
			}
		}
		return;
	}

}

/*****************************************************************************/
/*                                                                           */
/* collide with mesh                                                         */
/*                                                                           */
/*****************************************************************************/

int Collision::collide(Object *object)
{
	
	num_contacts = 0;
	num_objects = 0;
	
	if(BSPTree::sectors_.size() == 0) return 0;
	if(object->pos_.in_sector_id_ == -1) return 0;
	
	num_surfaces = 0;
	
	// transform and copy triangles from the mesh in to collide surface
	if(object->type_ == Object::OBJ_GEOMETRY) {
		
		Mesh *mesh = reinterpret_cast<OGeometry*>(object)->mesh_;
		
		for(int i = 0; i < mesh->getNumSurfaces(); i++)
		{
			surfaces[num_surfaces].num_triangles = mesh->getNumTriangles(i);
			
			if(surfaces[num_surfaces].num_triangles > NUM_TRIANGLES) {
				fprintf(stderr,"Collision::collide(): many vertex %d\n",surfaces[num_surfaces].num_triangles);
				continue;
			}
			
			Mesh::Triangle *t = mesh->getTriangles(i);
			Triangle *ct = surfaces[num_surfaces].triangles;
			
			Vector3 min = Vector3(1000000,1000000,1000000);	// calculate bound box
			Vector3 max = Vector3(-1000000,-1000000,-1000000);
			
			for(int j = 0; j < surfaces[num_surfaces].num_triangles; j++) {
				ct[j].v[0] = object->transform_.multiplyVec3(t[j].v[0]);
				ct[j].v[1] = object->transform_.multiplyVec3(t[j].v[1]);
				ct[j].v[2] = object->transform_.multiplyVec3(t[j].v[2]);
				Matrix4 rotate = object->transform_.getRotationPart();
				Vector3 normal = rotate.multiplyVec3(Vector3(t[j].plane.getVector3()));
				ct[j].plane = Vector4(normal, ct[j].v[0].negate().dot(normal));
				normal = rotate.multiplyVec4(Vector4(t[j].c[0])).getVector3();
				ct[j].c[0] = Vector4(normal,ct[j].v[0].negate().dot(normal));

				normal = rotate.multiplyVec4(Vector4(t[j].c[1])).getVector3();
				ct[j].c[1] = Vector4(normal,ct[j].v[1].negate().dot(normal));
				normal = rotate.multiplyVec4(Vector4(t[j].c[2])).getVector3();
				ct[j].c[2] = Vector4(normal,ct[j].v[2].negate().dot(normal));
				for(int k = 0; k < 3; k++) 
				{
					if(min['x'] > ct[j].v[k]['x']) min.set(ct[j].v[k]['x'], 'x');
					else if(max['x'] < ct[j].v[k]['x']) max.set(ct[j].v[k]['x'], 'x');
					if(min['y'] > ct[j].v[k]['y']) min.set(ct[j].v[k]['y'], 'y');
					else if(max['y'] < ct[j].v[k]['y']) max.set(ct[j].v[k]['y'], 'y');
					if(min['z'] > ct[j].v[k]['z']) min.set(ct[j].v[k]['z'], 'z');
					else if(max['z'] < ct[j].v[k]['z']) max.set(ct[j].v[k]['z'], 'z');
				}
			}
			// bound sphere
			surfaces[num_surfaces].center = (max + min).scale(1.0f / 2.0f);
			surfaces[num_surfaces].radius = (max - min).getLength() / 2.0f;
			// bound box
			surfaces[num_surfaces].min = min;
			surfaces[num_surfaces].max = max;
		
			// new surface
			num_surfaces++;
			if(num_surfaces == NUM_SURFACES) 
			{
				fprintf(stderr,"Collision::collide(): many surfaces %d\n",NUM_SURFACES);
				break;
			}
		}
	} 
	else 
	{
		fprintf(stderr,"Collision::collide(): %d format isn`t supported\n",object->type_);
	}
	
	if(!num_surfaces) return 0;
	
	// static objects
	for(int i = 0; i < object->pos_.sectors_.size(); i++) 
	{
		Sector *s = &BSPTree::sectors_[object->pos_.sectors_[i]];
		for(int j = 0; j < s->node_objects_.size(); j++) 
		{
			Object *o = s->node_objects_[j];
			if((o->getCenter() - object->pos_.getPosCoord() - object->getCenter()).getLength() 
				>= o->getRadius() + object->getRadius()) continue;
			collideObjectMesh(o);
		}
	}
	
	// dynamic objects
	for(int i = 0; i < object->pos_.sectors_.size(); i++) 
	{
		Sector *s = &BSPTree::sectors_[object->pos_.sectors_[i]];
		
		for(int j = 0; j < s->objects_.size(); j++) 
		{
			Object *o = s->objects_[j];
			if(object == o) continue;
			
			if((o->pos_.getPosCoord() + o->getCenter() - object->pos_.getPosCoord() - object->getCenter()).getLength() 
				>= o->getRadius() + object->getRadius()) continue;
			Matrix4 transform = o->itransform_.multiplyMat(object->transform_);
			
		/*	if(object->rigidbody_ && object->rigidbody_->num_joints > 0 && o->rigidbody_) 
			{
				RigidBody *rb = object->rigidbody_;
				int k;
				for(k = 0; k < rb->num_joints; k++) {
					if(rb->joined_rigidbodies[k] == o->rigidbody_) break;
				}
				if(k != rb->num_joints) continue;
			}*/
			
			// object bound box
			Vector3 object_min = Vector3(1000000,1000000,1000000);
			Vector3 object_max = Vector3(-1000000,-1000000,-1000000);
			
			// transform vertex and calcualte bound box
			num_surfaces = 0;
			if(object->type_ == Object::OBJ_GEOMETRY) 
			{
				Mesh *mesh = reinterpret_cast<OGeometry*>(object)->mesh_;
				for(int k = 0; k < mesh->getNumSurfaces(); k++) 
				{
					surfaces[num_surfaces].num_triangles = mesh->getNumTriangles(k);
					if(surfaces[num_surfaces].num_triangles > NUM_TRIANGLES) continue;
					
					Mesh::Triangle *t = mesh->getTriangles(k);
					Triangle *ct = surfaces[num_surfaces].triangles;
					
					Vector3 min = Vector3(1000000,1000000,1000000);	// calculate bound box
					Vector3 max = Vector3(-1000000,-1000000,-1000000);
					for(int l = 0; l < surfaces[num_surfaces].num_triangles; l++) {
						ct[l].v[0] = transform.multiplyVec3(t[l].v[0]);
						ct[l].v[1] = transform.multiplyVec3(t[l].v[1]);
						ct[l].v[2] = transform.multiplyVec3(t[l].v[2]);
						for(int m = 0; m < 3; m++) 
						{
							if(min['x'] > ct[l].v[m]['x']) min.set(ct[l].v[m]['x'], 'x');
							else if(max['x'] < ct[l].v[m]['x']) max.set(ct[l].v[m]['x'], 'x');
							if(min['y'] > ct[l].v[m]['y']) min.set(ct[l].v[m]['y'], 'y');
							else if(max['y'] < ct[l].v[m]['y']) max.set(ct[l].v[m]['y'], 'y');
							if(min['z'] > ct[l].v[m]['z']) min.set(ct[l].v[m]['z'], 'z');
							else if(max['z'] < ct[l].v[m]['z']) max.set(ct[l].v[m]['z'], 'z');
						}
					}
					// bound sphere
					surfaces[num_surfaces].center = (max + min).scale(1.0f/ 2.0f);
					surfaces[num_surfaces].radius = (max - min).getLength() / 2.0f;
					// bound box
					surfaces[num_surfaces].min = min;
					surfaces[num_surfaces].max = max;
					// object bound box
					if(object_min['x'] > min['x']) object_min.set(min['x'], 'x');
					if(object_max['x'] < max['x']) object_max.set(max['x'], 'x');
					if(object_min['y'] > min['y']) object_min.set(min['y'], 'y');
					if(object_max['y'] < max['y']) object_max.set(max['y'], 'y');
					if(object_min['z'] > min['z']) object_min.set(min['z'], 'z');
					if(object_max['z'] < max['z']) object_max.set(max['z'], 'z');
					
					num_surfaces++;
					if(num_surfaces == NUM_SURFACES) break;
				}
			}
			
			if(num_surfaces == 0) continue;
			
			// bound box test
			const Vector3 &min = o->getMin();
			const Vector3 &max = o->getMax();
			
			if(min['x'] >= object_max['x']) continue;
			if(max['x'] <= object_min['x']) continue;
			if(min['y'] >= object_max['y']) continue;
			if(max['y'] <= object_min['y']) continue;
			if(min['z'] >= object_max['z']) continue;
			if(max['z'] <= object_min['z']) continue;
			
			// recalculate normals and fast point and triangle
			num_surfaces = 0;
			if(object->type_ == Object::OBJ_GEOMETRY) 
			{
				Mesh *mesh = reinterpret_cast<OGeometry*>(object)->mesh_;
				for(int k = 0; k < mesh->getNumSurfaces(); k++)
				{
					surfaces[num_surfaces].num_triangles = mesh->getNumTriangles(k);
					if(surfaces[num_surfaces].num_triangles > NUM_TRIANGLES) continue;
					
					Mesh::Triangle *t = mesh->getTriangles(k);
					Triangle *ct = surfaces[num_surfaces].triangles;
					
					for(int l = 0; l < surfaces[num_surfaces].num_triangles; l++) {
						Matrix4 rotate = transform.getRotationPart();
						Vector3 normal = rotate.multiplyVec3(Vector3(t[l].plane.getVector3()));
						ct[l].plane = Vector4(normal,ct[l].v[0].negate().dot(normal));
						normal = rotate.multiplyVec4(Vector4(t[l].c[0])).getVector3();
						ct[l].c[0] = Vector4(normal,ct[l].v[0].negate().dot(normal));
						normal = rotate.multiplyVec4(Vector4(t[l].c[1])).getVector3();
						ct[l].c[1] = Vector4(normal,ct[l].v[1].negate().dot(normal));
						normal = rotate.multiplyVec4(Vector4(t[l].c[2])).getVector3();
						ct[l].c[2] = Vector4(normal,ct[l].v[2].negate().dot(normal));
					}
					
					num_surfaces++;
					if(num_surfaces == NUM_SURFACES) break;
				}
			}
			if(num_surfaces == 0) continue;
			
			// collide it
			collideObjectMesh(o);
		}
	}
	
	return num_contacts;
}

/*
 */
void Collision::collideObjectMesh(Object *object) 
{
	
	static int next[3] = { 1, 2, 0 };
	
	// collide Mesh-Mesh
	if(object->type_ == Object::OBJ_GEOMETRY) 
	{
		Mesh *mesh = reinterpret_cast<OGeometry*>(object)->mesh_;
		
		for(int i = 0; i < mesh->getNumSurfaces(); i++) 
		{
			for(int j = 0; j < num_surfaces; j++) {
				
				if((mesh->getCenter(i) - surfaces[j].center).getLength() 
					>= mesh->getRadius(i) + surfaces[j].radius) continue;
				
				const Vector3 &min = mesh->getMin(i);
				const Vector3 &max = mesh->getMax(i);
				
				if(min['x'] >= surfaces[j].max['x']) continue;
				if(max['x'] <= surfaces[j].min['x']) continue;
				if(min['y'] >= surfaces[j].max['y']) continue;
				if(max['y'] <= surfaces[j].min['y']) continue;
				if(min['z'] >= surfaces[j].max['z']) continue;
				if(max['z'] <= surfaces[j].min['z']) continue;
				
				Mesh::Triangle *triangles = mesh->getTriangles(i);
				Surface *s = &surfaces[j];
				
				int num_triangles = mesh->getNumTriangles(i);
				for(int k = 0; k < num_triangles; k++) 
				{
					
					Mesh::Triangle *t = &triangles[k];
					
					for(int l = 0; l < s->num_triangles; l++) 
					{	
						Triangle *ct = &s->triangles[l];
						
						// fast triangle-triangle intersection test
						float dist0[3];
						int collide0 = 0;
						for(int m = 0; m < 3; m++) 
						{
							dist0[m] = t->plane.dotVec3(ct->v[m]);  // ...+ ..+ ..+ w
							if(fabs(dist0[m]) > s->radius * 2) {
								collide0 = 3;
								break;
							}
							if(dist0[m] > 0.0) collide0++;
							else if(dist0[m] < 0.0) collide0--;
						}
						if(collide0 == 3 || collide0 == -3) continue;
						
						for(int m = 0; m < 3; m++)
						{
							if(!((dist0[m] <= 0.0) ^ (dist0[next[m]] <= 0.0))) continue;
							
							Vector3 &v0 = ct->v[m];
							Vector3 &v1 = ct->v[next[m]];
							Vector3 edge = v1 - v0;
							Vector3 p = v0 - edge.scale(dist0[m]).scale(1.0f / (dist0[next[m]] - dist0[m]));	// intersect ct edge with t plane
							
							int p_inside = t->c[0].dotVec3(p) > 0.0 && t->c[1].dotVec3(p) > 0.0 && t->c[2].dotVec3(p) > 0.0;
							if(!p_inside) continue;
							
							int v0_inside = t->c[0].dotVec3(v0) > 0.0 && t->c[1].dotVec3(v0) > 0.0 && t->c[2].dotVec3(v0) > 0.0;
							int v1_inside = t->c[0].dotVec3(v1) > 0.0 && t->c[1].dotVec3(v1) > 0.0 && t->c[2].dotVec3(v1) > 0.0;
							
							if(v0_inside && v1_inside)
							{
								float length = edge.getLength() / 2.0f;
								if(dist0[m] < 0.0 && dist0[m] > -length) {
									if(!addContact(object,object->materials_[i],v0,t->plane.getVector3(),-dist0[m],true)) return;
								} else if(dist0[next[m]] < 0.0 && dist0[next[m]] > -length) {
									if(!addContact(object,object->materials_[i],v1,t->plane.getVector3(),-dist0[next[m]],true)) return;
								}
							}
							
							if(v0_inside != v1_inside) 
							{
								float dist = 1000000;
								for(int n = 0; n < 3; n++)
								{
									float d = t->c[n].dotVec3(p);
									if(dist > d) dist = d;
								}
								if(dist0[m] < 0.0) {
									float d = (v0 - p).getLength();
									if(dist > d) dist = 0;
									else dist = -dist0[m] * dist / d;
								} else {
									float d = (v1 - p).getLength();
									if(dist > d) dist = 0;
									else dist = -dist0[next[m]] * dist / d;
								}
								if(!addContact(object,object->materials_[i], p, ct->plane.negate().getVector3(), dist)) return;
							}
						}
						
						// collide with scene
						if(!object->rigidbody_) 
						{
							float dist1[3];
							int collide1 = 0;
							for(int m = 0; m < 3; m++) 
							{
								dist1[m] = ct->plane.dotVec3(t->v[m]);
								if(dist1[m] > 0.0) collide1++;
								else if(dist1[m] < 0.0) collide1--;
							}
							if(collide1 == 3 || collide1 == -3) continue;
						
							for(int m = 0; m < 3; m++) {
								if(dist1[m] >= 0) continue;
								if(dist1[next[m]] <= 0) continue;

								Vector3 &v0 = t->v[m];
								Vector3 &v1 = t->v[next[m]];
								Vector3 edge = v1 - v0;
								Vector3 p = v0 - edge.scale(dist1[m]).scale(1.0f / (dist1[next[m]] - dist1[m]));	// intersect ct edge with t plane
								
								int p_inside = ct->c[0].dotVec3(p) > 0.0 && ct->c[1].dotVec3(p) > 0.0 && ct->c[2].dotVec3(p) > 0.0;
								if(!p_inside) continue;
								
								/*	int v0_inside = v0 * ct->c[0] > 0.0 && v0 * ct->c[1] > 0.0 && v0 * ct->c[2] > 0.0;
								int v1_inside = v1 * ct->c[0] > 0.0 && v1 * ct->c[1] > 0.0 && v1 * ct->c[2] > 0.0;*/

								int v0_inside = ct->c[0].dotVec3(v0) > 0.0 && ct->c[1].dotVec3(v0) > 0.0 && ct->c[2].dotVec3(v0) > 0.0;
								int v1_inside = ct->c[0].dotVec3(v1) > 0.0 && ct->c[1].dotVec3(v1) > 0.0 && ct->c[2].dotVec3(v1) > 0.0;
								
								if(v0_inside != v1_inside) 
								{
									float dist = 1000000;
									for(int n = 0; n < 3; n++) {
										float d = ct->c[n].dotVec3(p);
										if(dist > d) dist = d;
									}
									float d = (v0 - p).getLength();
									if(dist > d) dist = 0;
									else dist = -dist1[m] * dist / d;
									if(!addContact(object,object->materials_[i],p,t->plane.getVector3(),dist)) return;
								}
							}
						}
					}
				}
			}
		}
		return;
	}
}

/*****************************************************************************/
/*                                                                           */
/* sorting contacts                                                          */
/*                                                                           */
/*****************************************************************************/

static int contact_cmp(const void *a,const void *b) 
{
	Collision::Contact *c0 = (Collision::Contact*)a;
	Collision::Contact *c1 = (Collision::Contact*)b;
	if(c0->object->pos_.getPosCoord()['z'] > c1->object->pos_.getPosCoord()['z']) return 1;
	if(c0->object->pos_.getPosCoord()['z'] < c1->object->pos_.getPosCoord()['z']) return -1;
	if(c0->point['z'] > c1->point['z']) return 1;
	if(c0->point['z'] < c1->point['z']) return -1;
	return 0;
}

void Collision::sort() 
{
	qsort(contacts,num_contacts,sizeof(Contact),contact_cmp);
}
