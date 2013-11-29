#include "BSPTree.h"
#include "Mesh.h"
#include "MeshVBO.h"
#include "OGeometry.h"
#include "assert.h"
#include "core.h"


/*****************************************************************************/
/*                                                                           */
/* Node                                                                      */
/*                                                                           */
/*****************************************************************************/

Node::Node() 
	: left_(nullptr), 
	  right_(nullptr), 
	  object_(nullptr), 
	  bbox_(), 
	  bsphere_()
{
}

Node::~Node() 
{
	if(left_)
	{
		delete left_;
	}
	if(right_)
	{
		delete right_;
	}

	if(object_)
	{
		delete object_;
	}
}

/*
 */
void Node::create(Mesh *mesh) 
{
	bbox_.set(mesh->getMin(), mesh->getMax());
	bsphere_.set(mesh->getCenter(), mesh->getRadius());

	int num_vertex = 0;
	for(int i = 0; i < mesh->getNumSurfaces(); i++)
	{
		num_vertex += mesh->getNumVertex(i);
	}

	if(num_vertex / 3 > TRIANGLES_PER_NODE) 
	{
		Vector4 plane;
		Vector3 size = bbox_.getMax() - bbox_.getMin();
		if(size['x'] > size['y']) 
		{	// find clip plane
			if(size['x'] > size['z'])
			{
				plane = Vector4(1,0,0,1);
			}
			else
			{
				plane = Vector4(0,0,1,1);
			}

		} 
		else
		{
			if(size['y'] > size['z'])
			{
				plane = Vector4(0,1,0,1);
			}
			else
			{
				plane = Vector4(0,0,1,1);
			}
		}

		Vector3 center(0,0,0);
		int num_vertex = 0;	// find center of node
		for(int i = 0; i < mesh->getNumSurfaces(); i++) 
		{
			Mesh::Vertex *vertex = mesh->getVertex(i);
			for(int j = 0; j < mesh->getNumVertex(i); j++) 
			{
				center = center + vertex[j].xyz;
				num_vertex++;
			}
		}

		float oneDivNumVtx = 1.0f / (float)num_vertex;
		//center /= num_vtx;
		center = center.scale(oneDivNumVtx);

		plane.set( -(plane.dot(Vector4(center['x'], center['y'], center['z'], 1))), 'w');

		Mesh *left_mesh  = new Mesh();
		Mesh *right_mesh = new Mesh();

		for(int i = 0; i < mesh->getNumSurfaces(); i++) 
		{
			int num_vertex = mesh->getNumVertex(i);
			Mesh::Vertex *vertex = mesh->getVertex(i);

			int left_mesh_num_vertex = 0;
			Mesh::Vertex *left_mesh_vertex  = new Mesh::Vertex[num_vertex * 2];

			int right_mesh_num_vertex = 0;
			Mesh::Vertex *right_mesh_vertex = new Mesh::Vertex[num_vertex * 2];

			for(int j = 0; j < num_vertex; j += 3) 
			{
				int num_left = 0;
				int num_right = 0;
				Mesh::Vertex *left  = &left_mesh_vertex[left_mesh_num_vertex];
				Mesh::Vertex *right = &right_mesh_vertex[right_mesh_num_vertex];

				Vector3 &xyz0 = vertex[j + 0].xyz;
				float cur_dot = plane * Vector4(xyz0['x'], xyz0['y'], xyz0['z'], 1);

				for(int cur = 0; cur < 3; cur++) 
				{
					int next = (cur + 1) % 3;
					Vector3 &xyz_next = vertex[j + next].xyz;
					float next_dot = plane * Vector4(xyz_next['x'], xyz_next['y'], xyz_next['z'], 1);

					if(cur_dot <= 0.0)
					{
						left[num_left++] = vertex[j + cur];
					}
					if(cur_dot > 0.0)
					{
						right[num_right++] = vertex[j + cur];
					}
					if((cur_dot <= 0.0) != (next_dot <= 0.0)) 
					{
						float k = -cur_dot / (next_dot - cur_dot);
						Mesh::Vertex v;
						v.xyz      = vertex[j + cur].xyz.scale(1.0f - k)      + vertex[j + next].xyz.scale(k);
						v.normal   = vertex[j + cur].normal.scale(1.0f - k)   + vertex[j + next].normal.scale(k);
						v.tangent  = vertex[j + cur].tangent.scale(1.0f - k)  + vertex[j + next].tangent.scale(k);
						v.binormal = vertex[j + cur].binormal.scale(1.0f - k) + vertex[j + next].binormal.scale(k);
						v.texcoord = vertex[j + cur].texcoord.scale(1.0f - k) + vertex[j + next].texcoord.scale(k);

						left[num_left++]   = v;
						right[num_right++] = v;
					}
					cur_dot = next_dot;
				}
				if(num_left == 3)
				{
					left_mesh_num_vertex += 3;
				}
				else if(num_left == 4) 
				{
					left[4] = left[0];
					left[5] = left[2];
					left_mesh_num_vertex += 6;
				}
				if(num_right == 3)
				{
					right_mesh_num_vertex += 3;
				}
				else if(num_right == 4) 
				{
					right[4] = right[0];
					right[5] = right[2];
					right_mesh_num_vertex += 6;
				}
			}
			if(left_mesh_num_vertex > 0)
			{
				left_mesh->addSurface(mesh->getSurfaceName(i),left_mesh_vertex,left_mesh_num_vertex);
			}
			if(right_mesh_num_vertex > 0)
			{
				right_mesh->addSurface(mesh->getSurfaceName(i),right_mesh_vertex,right_mesh_num_vertex);
			}
			delete right_mesh_vertex;
			delete left_mesh_vertex;
		}

		int left_mesh_num_vertex = 0;
		for(int i = 0; i < left_mesh->getNumSurfaces(); i++)
		{
			left_mesh_num_vertex += left_mesh->getNumVertex(i);
		}
		int right_mesh_num_vertex = 0;
		for(int i = 0; i < right_mesh->getNumSurfaces(); i++)
		{
			right_mesh_num_vertex += right_mesh->getNumVertex(i);
		}
		if(left_mesh_num_vertex > 0 && right_mesh_num_vertex > 0) 
		{
			left_mesh->create_mesh_bounds();
			left_ = new Node();
			left_->create(left_mesh);
			right_mesh->create_mesh_bounds();
			right_ = new Node();
			right_->create(right_mesh);
		} 
		else 
		{
			mesh->create_triangle_strips();
			object_ = new OGeometry(new MeshVBO(mesh));
			delete right_mesh;
			delete left_mesh;
		}
	} 
	else 
	{
		mesh->create_triangle_strips();
		object_ = new OGeometry(new MeshVBO(mesh));
	}
	delete mesh;
}

/*
 */
void Node::bindMaterial(const char *name, Material *material) 
{
	if(left) left->bindMaterial(name, material);
	if(right) right->bindMaterial(name, material);
	if(object) object->bindMaterial(name, material);
}

/*
 */
void Node::render() 
{
	if(left && right) 
	{
		int check_left = Engine::frustum->inside(left->center,left->radius);
		int check_right = Engine::frustum->inside(right->center,right->radius);
		if(check_left && check_right) 
		{
			if((left->center - Engine::camera).length() < (right->center - Engine::camera).length()) 
			{
				left->render();
				right->render();
			} else 
			{
				right->render();
				left->render();
			}
			return;
		}
		if(check_left) left->render();
		else if(check_right) right->render();
		return;
	}
	if(object && object->frame != Engine::frame) 
	{
		Sector *s = BSPTree::visible_sectors[BSPTree::num_visible_sectors - 1];
		s->visible_objects[s->num_visible_objects++] = object;
		Engine::num_triangles += object->render(Object::RENDER_OPACITY);
	}
}

/*****************************************************************************/
/*                                                                           */
/* Portal                                                                    */
/*                                                                           */
/*****************************************************************************/

Portal::Portal() 
	: bsphere_(),
	  sectors_(), 
	  frame_(0) 
{
	
}

Portal::~Portal() 
{
}

/*
 */
void Portal::create(Mesh *mesh, int surface_id)
{
	bsphere_.set(mesh->getCenter(surface_id), mesh->getRadius(surface_id));
	int num_vertex = mesh->getNumVertex(surface_id);

	if(num_vertex != 6) 
	{
		fprintf(stderr,"Portal::create(): portal mesh must have only two triangle\n");
		return;
	}

	Mesh::Vertex *v = mesh->getVertex(surface_id);
	int flag[6];	// create quad from the six vertexes
	for(int i = 0; i < 6; i++) 
	{
		flag[i] = 0;
		for(int j = 0; j < 6; j++) 
		{
			if(i == j)
			{
				continue;
			}
			if(v[i].xyz == v[j].xyz)
			{
				flag[i] = 1;
			}
		}
	}
	for(int i = 0, j = 0; i < 6; i++) 
	{
		if(flag[i] == 0)
		{
			points_[j++] = v[i].xyz;
		}
		if(i == 5 && j != 2) 
		{
			fprintf(stderr,"Portal::create(): can`t find two similar vertexes for create quad\n");
			return;
		}
	}
	points_[2] = points_[1];
	for(int i = 0, j = 1; i < 3; i++) 
	{
		if(flag[i] != 0) 
		{
			points_[j] = v[i].xyz;
			j += 2;
		}
	}
}

void Portal::getScissor(int *scissor) 
{
	if((bsphere_.getCenter().getVector3() - Core::camera_).getLength() < bsphere_.getRadius()) 
	{
		scissor[0] = Core::viewport_[0];
		scissor[1] = Core::viewport_[1];
		scissor[2] = Core::viewport_[2];
		scissor[3] = Core::viewport_[3];
		return;
	}
	Matrix4 mvp = Core::projection_.multiplyMat(Core::modelview_);
	Vector4 p[4];

	p[0] = mvp.multiplyVec4(Vector4(points_[0]['x'], points_[0]['y'], points_[0]['z'], 1));
	p[1] = mvp.multiplyVec4(Vector4(points_[1]['x'], points_[1]['y'], points_[1]['z'], 1));
	p[2] = mvp.multiplyVec4(Vector4(points_[2]['x'], points_[2]['y'], points_[2]['z'], 1));
	p[3] = mvp.multiplyVec4(Vector4(points_[3]['x'], points_[3]['y'], points_[3]['z'], 1));
	p[0] = p[0].scale(1.0f / p[0]['w']);
	p[1] = p[1].scale(1.0f / p[1]['w']);
	p[2] = p[2].scale(1.0f / p[2]['w']);
	p[3] = p[3].scale(1.0f / p[3]['w']);

	Vector3 min = Vec3_INF;
	Vector3 max = Vec3_INF.negate();
	for(int i = 0; i < 4; i++) 
	{
		if(min['x'] > p[i]['x']) min.set(p[i]['x'], 'x');
		if(max['x'] < p[i]['x']) max.set(p[i]['x'], 'x');
		if(min['y'] > p[i]['y']) min.set(p[i]['y'], 'y');
		if(max['y'] < p[i]['y']) max.set(p[i]['y'], 'y');
	}
	scissor[0] = Core::viewport_[0] + (int)((float)Core::viewport_[2] * (min['x'] + 1.0) / 2.0) - 4;	// remove it
	scissor[1] = Core::viewport_[1] + (int)((float)Core::viewport_[3] * (min['y'] + 1.0) / 2.0) - 4;
	scissor[2] = Core::viewport_[0] + (int)((float)Core::viewport_[2] * (max['x'] + 1.0) / 2.0) + 4;
	scissor[3] = Core::viewport_[1] + (int)((float)Core::viewport_[3] * (max['y'] + 1.0) / 2.0) + 4;

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

/*
 */
void Portal::render() {
	glBegin(GL_QUADS);
	{
		glVertex3fv(points_[0]);
		glVertex3fv(points_[1]);
		glVertex3fv(points_[2]);
		glVertex3fv(points_[3]);
	}
	glEnd();
}

/*****************************************************************************/
/*                                                                           */
/* Sector                                                                    */
/*                                                                           */
/*****************************************************************************/

Sector::Sector() 
	: bsphere_(), 
	  planes_(), 
	  root(nullptr),
	  portals_(), 
	  objects_(), 
	  node_objects_(), 
	  visible_objects_(), 
	  portal_(nullptr), 
	  frame_(0), 
	  old_visible_objects_(), 
	  old_portal_(nullptr), 
	  old_frame_(0) 
{
	
}

Sector::~Sector() 
{
	if(root) delete root;
	if(portal_) delete portal_;
	if(old_portal_) delete old_portal_;
	for(int i = 0; i < node_objects_.size(); i++)
	{
		delete node_objects_[i];
	}
	for(int i = 0; i < visible_objects_.size(); i++)
	{
		delete visible_objects_[i];
	}
	for(int i = 0; i < old_visible_objects_.size(); i++)
	{
		delete old_visible_objects_[i];
	}
}

/*
 */
void Sector::create(Mesh *mesh, int surface_id) 
{
	bsphere_.set(mesh->getCenter(surface_id), mesh->getRadius(surface_id));
	planes_.resize(mesh->getNumVertex(surface_id) / 3);

	Mesh::Vertex *v = mesh->getVertex(surface_id);
	for(int i = 0; i < planes_.size(); i++) 
	{
		Vector3 normal;
		normal = Vector3::Cross(v[i * 3 + 1].xyz - v[i * 3 + 0].xyz,v[i * 3 + 2].xyz - v[i * 3 + 0].xyz);
		normal.normalize();
		
		planes_[i] = Vector4(normal['x'], normal['y'], normal['z'], normal.negate().dot(v[i * 3 + 0].xyz));
	}
}

/* get all objects from the tree
 */
void Sector::getNodeObjects(Node *node) 
{
	if(node->left_ && node->right_)
	{
		getNodeObjects(node->left_);
		getNodeObjects(node->right_);
	}
	if(node->object_) 
	{
		node_objects_.push_back(node->object_);
	}
}

/*
 */
void Sector::create() 
{
	objects_.resize(NUM_OBJECTS);
	//fill node_objects_
	getNodeObjects(root);
	
	visible_objects_.resize(node_objects_.size()     + NUM_OBJECTS);
	old_visible_objects_.resize(node_objects_.size() + NUM_OBJECTS);
}

/*
 */
int Sector::inside(const Vector3 &point) 
{
	for(int i = 0; i < planes_.size(); i++) 
	{
		if(planes_[i].dot(Vector4(point['x'], point['y'], point['z'], 1) > 0.0))
		{
			return 0;
		}
	}

	return 1;
}

int Sector::inside(Portal *portal) 
{
	if(inside(portal->points_[0]) == 0) return 0;
	if(inside(portal->points_[1]) == 0) return 0;
	if(inside(portal->points_[2]) == 0) return 0;
	if(inside(portal->points_[3]) == 0) return 0;
	return 1;
}

int Sector::inside(const BSphere &bsphere_) 
{
	for(int i = 0; i < planes_.size(); i++) 
	{
		Vector3 center_pos = bsphere_.getCenter().getVector3();
		Vector4 temp = Vector4(center_pos['x'], center_pos['y'], center_pos['z'], 1);
		if(planes_[i].dot(temp) > bsphere_.getRadius()) 
		{
			return 0;
		}
	}
	return 1;
}

int Sector::inside(Mesh *mesh, int surface_id) 
{
	Mesh::Vertex *v = mesh->getVertex(surface_id);
	for(int i = 0; i < mesh->getNumVertex(surface_id); i++) 
	{
		if(inside(v[i].xyz) == 0) return 0;
	}
	return 1;
}

/*
 */
void Sector::addObject(Object *object) 
{
	if(objects_.empty()) return;
	for(int i = 0; i < objects_.size(); i++)
	{
		if(objects_[i] == object)
		{
			return;
		}
	}
	objects_.push_back(object);
}

/*
 */
void Sector::removeObject(Object *object) 
{
	for(int i = 0; i < objects_.size(); i++) 
	{
		if(objects_[i] == object) 
		{
			objects_.erase(objects_.begin() + i);
			return;
		}
	}
}

/*
 */
void Sector::bindMaterial(const char *name, Material *material) 
{
	root->bindMaterial(name, material);
}

/*
 */
void Sector::render(Portal *portal) 
{
	if(frame_ == Core::curr_frame_)
	{
		return;
	}
	frame_ = Core::curr_frame_;
	
	portal_ = portal;

	BSPTree::visible_sectors_.push_back(this);
	assert(BSPTree::visible_sectors_.size() <= BSPTree::sectors_.size() 
		&& "visible_sectors shouldn't exceed the total number of sectors");
	
	root->render();
	
	for(int i = 0; i < objects_.size(); i++) 
	{
		Object *o = objects_[i];
		if(o->frame_ == Core::curr_frame_)
		{
			continue;
		}

		if(Core::frustum_->inside(o->pos_.getPosCoord() + o->getCenter(), o->getRadius())) 
		{
			Core::num_triangles_ += o->render();
			visible_objects_.push_back(o);
		}
	}
	
	for(int i = 0; i < portals_.size(); i++) 
	{
		Portal *p = &BSPTree::portals_[portals_[i]];
		if(p->frame_ == Core::curr_frame_)
		{
			continue;
		}

		p->frame_ = Core::curr_frame_;

		if(Core::frustum_->inside(p->bsphere_.getCenter().getVector3(), p->bsphere_.getRadius())) {
			float dist = (Core::camera_ - p->bsphere_.getCenter().getVector3()).getLength();
			
			if(Core::have_occlusion_ && dist > p->bsphere_.getRadius()) 
			{
				if(Material::old_material_)
				{
					Material::old_material_->disable();
				}
				
				glDisable(GL_CULL_FACE);
				glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
				glDepthMask(GL_FALSE);
				glBeginQueryARB(GL_SAMPLES_PASSED_ARB, Core::o_query_id_);
				p->render();
				glEndQueryARB(GL_SAMPLES_PASSED_ARB);
				glDepthMask(GL_TRUE);
				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
				glEnable(GL_CULL_FACE);
					
				GLuint samples;
				glGetQueryObjectuivARB(Core::o_query_id_, GL_QUERY_RESULT_ARB, &samples);
				if(samples == 0)
				{
					continue;
				}
			}
			
			if(dist > p->bsphere_.getRadius())
			{
				Core::frustum_->addPortal(Core::camera_.getPosCoord(), p->points_);
			}

			for(int j = 0; j < p->sectors_.size(); j++) 
			{
				BSPTree::sectors_[p->sectors_[j]].render(dist > p->bsphere_.getRadius() ? p : NULL);
			}

			if(dist > p->bsphere_.getRadius())
			{
				Core::frustum_->removePortal();
			}
		}
	}
}

/*
 */
void Sector::saveState() {
	old_num_visible_objects = num_visible_objects;
	for(int i = 0; i < num_visible_objects; i++) {
		old_visible_objects[i] = visible_objects[i];
	}
	old_portal = portal;
	old_frame = frame;
}

void Sector::restoreState(int frame) {
	num_visible_objects = old_num_visible_objects;
	for(int i = 0; i < num_visible_objects; i++) {
		visible_objects[i] = old_visible_objects[i];
	}
	portal = old_portal;
	this->frame = frame;
}

/*****************************************************************************/
/*                                                                           */
/* BSPTree                                                                       */
/*                                                                           */
/*****************************************************************************/

int BSPTree::num_portals;
Portal *BSPTree::portals;
int BSPTree::num_sectors;
Sector *BSPTree::sectors;
int BSPTree::num_visible_sectors;
Sector **BSPTree::visible_sectors;
int BSPTree::old_num_visible_sectors;
Sector **BSPTree::old_visible_sectors;

BSPTree::BSPTree() {
	num_portals = 0;
	portals = NULL;
	num_sectors = 0;
	sectors = NULL;
	num_visible_sectors = 0;
	visible_sectors = NULL;
	old_num_visible_sectors = 0;
	old_visible_sectors = NULL;
}

BSPTree::~BSPTree() {
	if(num_portals) delete [] portals;
	num_portals = 0;
	portals = NULL;
	if(num_sectors) delete [] sectors;
	num_sectors = 0;
	sectors = NULL;
	if(visible_sectors) delete visible_sectors;
	num_visible_sectors = 0;
	visible_sectors = NULL;
	if(old_visible_sectors) delete old_visible_sectors;
	old_num_visible_sectors = 0;
	old_visible_sectors = NULL;
}

/*****************************************************************************/
/*                                                                           */
/* BSPTree IO                                                                    */
/*                                                                           */
/*****************************************************************************/

/*
 */
void BSPTree::load(const char *name) {
	
	if(strstr(name,".bsp")) {	// read own binary bsp format
		
		FILE *file = fopen(name,"rb");
		if(!file) {
			fprintf(stderr,"BSPTree::load(): error open \"%s\" file\n",name);
			return;
		}
		
		int magic;
		fread(&magic,sizeof(int),1,file);
		if(magic != BSP_MAGIC) {
			fprintf(stderr,"BSPTree::load(): wrong magic in \"%s\" file\n",name);
			fclose(file);
			return;
		}
		
		fread(&num_portals,sizeof(int),1,file);
		portals = new Portal[num_portals];
		for(int i = 0; i < num_portals; i++) {
			Portal *p = &portals[i];
			fread(&p->center,sizeof(Vector3),1,file);
			fread(&p->radius,sizeof(float),1,file);
			fread(&p->num_sectors,sizeof(int),1,file);
			p->sectors = new int[p->num_sectors];
			fread(p->sectors,sizeof(int),p->num_sectors,file);
			fread(p->points,sizeof(Vector3),4,file);
		}
		
		fread(&num_sectors,sizeof(int),1,file);
		sectors = new Sector[num_sectors];
		for(int i = 0; i < num_sectors; i++) {
			Sector *s = &sectors[i];
			fread(&s->center,sizeof(Vector3),1,file);
			fread(&s->radius,sizeof(float),1,file);
			fread(&s->num_portals,sizeof(int),1,file);
			s->portals = new int[s->num_portals];
			fread(s->portals,sizeof(int),s->num_portals,file);
			fread(&s->num_planes,sizeof(int),1,file);
			s->planes = new Vector4[s->num_planes];
			fread(s->planes,sizeof(Vector4),s->num_planes,file);
			s->root = new Node();
			s->root->load(file);
			s->create();
		}
		
		fclose(file);
		
		//visible_sectors = new Sector*[num_sectors];
		//old_visible_sectors = new Sector*[num_sectors];
		
		Engine::console->printf("sectors %d\nportals %d\n",num_sectors,num_portals);
		
		return;
	}
	
	// else generate bsp tree with portals and sectors
	Mesh *mesh = new Mesh();
	if(strstr(name,".3ds")) mesh->load_3ds(name);
	else if(strstr(name,".mesh")) mesh->load_mesh(name);
	
	mesh->calculate_bounds();
	mesh->calculate_tangent();

	for(int i = 0; i < mesh->getNumSurfaces(); i++) {
		const char *name = mesh->getSurfaceName(i);
		if(!strncmp(name,"portal",6)) num_portals++;
		else if(!strncmp(name,"sector",6)) num_sectors++;
	}
	
	if(num_portals == 0 || num_sectors == 0) {
		
		num_sectors = 1;
		sectors = new Sector[1];
		sectors[0].root = new Node();
		sectors[0].root->create(mesh);
		sectors[0].create();
		
	} else {
		
		portals = new Portal[num_portals];
		sectors = new Sector[num_sectors];
		
		int *usage_flag = new int[mesh->getNumSurfaces()];
		
		num_portals = 0;
		num_sectors = 0;
		for(int i = 0; i < mesh->getNumSurfaces(); i++) {
			const char *name = mesh->getSurfaceName(i);
			if(!strncmp(name,"portal",6)) {
				portals[num_portals++].create(mesh,i);
				usage_flag[i] = 1;
			} else if(!strncmp(name,"sector",6)) {
				sectors[num_sectors++].create(mesh,i);
				usage_flag[i] = 1;
			} else usage_flag[i] = 0;
		}
		
		for(int i = 0; i < num_portals; i++) portals[i].sectors = new int[num_sectors];
		for(int i = 0; i < num_sectors; i++) sectors[i].portals = new int[num_portals];
		
		for(int i = 0; i < num_sectors; i++) {
			Sector *s = &sectors[i];
			for(int j = 0; j < num_portals; j++) {
				Portal *p = &portals[j];
				if(s->inside(p)) {
					p->sectors[p->num_sectors++] = i;
					s->portals[s->num_portals++] = j;
				}
			}
		}
		
		for(int i = 0; i < num_sectors; i++) {
			Sector *s = &sectors[i];
			Mesh *m = new Mesh();
			for(int j = 0; j < mesh->getNumSurfaces(); j++) {
				if(usage_flag[j]) continue;
				if(s->inside(mesh,j)) {
					m->addSurface(mesh,j);
					usage_flag[j] = 1;
				}
			}
			s->root = new Node();
			s->root->create(m);
			s->create();
		}
		
		delete usage_flag;
		delete mesh;
	}
	
	visible_sectors = new Sector*[num_sectors];
	old_visible_sectors = new Sector*[num_sectors];
	
	Engine::console->printf("sectors %d\nportals %d\n",num_sectors,num_portals);
}

/*
 */
void BSPTree::save(const char *name) {
	FILE *file = fopen(name,"wb");
	if(!file) {
		fprintf(stderr,"BSPTree::save(): can`t create \"%s\" file\n",name);
		return;
	}
	int magic = BSP_MAGIC;
	fwrite(&magic,sizeof(int),1,file);
	fwrite(&num_portals,sizeof(int),1,file);
	for(int i = 0; i < num_portals; i++) {
		Portal *p = &portals[i];
		fwrite(&p->center,sizeof(Vector3),1,file);
		fwrite(&p->radius,sizeof(float),1,file);
		fwrite(&p->num_sectors,sizeof(int),1,file);
		fwrite(p->sectors,sizeof(int),p->num_sectors,file);
		fwrite(p->points,sizeof(Vector3),4,file);
	}
	fwrite(&num_sectors,sizeof(int),1,file);
	for(int i = 0; i < num_sectors; i++) {
		Sector *s = &sectors[i];
		fwrite(&s->center,sizeof(Vector3),1,file);
		fwrite(&s->radius,sizeof(float),1,file);
		fwrite(&s->num_portals,sizeof(int),1,file);
		fwrite(s->portals,sizeof(int),s->num_portals,file);
		fwrite(&s->num_planes,sizeof(int),1,file);
		fwrite(s->planes,sizeof(Vector4),s->num_planes,file);
		s->root->save(file);
	}
	fclose(file);
}

/*****************************************************************************/
/*                                                                           */
/* BSPTree Render                                                                */
/*                                                                           */
/*****************************************************************************/

/*
 */
void BSPTree::bindMaterial(const char *name,Material *material) {
	for(int i = 0; i < num_sectors; i++) sectors[i].bindMaterial(name,material);
}

/*
 */
void BSPTree::render() {
	num_visible_sectors = 0;
	if(Engine::camera.sector != -1) {
		sectors[Engine::camera.sector].render();
	} else {
		int sector = -1;
		float dist = 1000000.0;
		for(int i = 0; i < num_sectors; i++) {
			float d = (sectors[i].center - Engine::camera).length();
			if(d < dist) {
				dist = d;
				sector = i;
			}
		}
		if(sector != -1) sectors[sector].render();
	}
}

/*
 */
void BSPTree::saveState() {
	old_num_visible_sectors = num_visible_sectors;
	for(int i = 0; i < num_visible_sectors; i++) {
		old_visible_sectors[i] = visible_sectors[i];
		old_visible_sectors[i]->saveState();
	}
}

void BSPTree::restoreState(int frame) {
	num_visible_sectors = old_num_visible_sectors;
	for(int i = 0; i < num_visible_sectors; i++) {
		visible_sectors[i] = old_visible_sectors[i];
		visible_sectors[i]->restoreState(frame);
	}
}
