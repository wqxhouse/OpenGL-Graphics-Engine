#include "WorldFile.h"

#include "parser.h"
#include "light.h"
#include "fog.h"
#include "mesh.h"
#include "OGeometry.h"
#include "rigidbody.h"
#include "particles.h"
#include "OParticles.h"
#include "core.h"

char *WorldFile::data;

/*
 */
void WorldFile::load(const char *name) 
{
	Parser *parser = new Parser(name);
	
	char file_name[1024];
	strcpy(file_name,name);
	
	if(parser->get("path")) 
	{
		char *s = parser->get("path");
		char path[1024];
		char *d = path;
		while(*s) {
			if(!strchr(" \t\n\r",*s)) *d++ = *s;
			s++;
		}
		*d = '\0';
		Core::AddDirectoryPath(path);
	}
	
	char *data_block = (char*)Parser::interpret(parser->get("data"));
	if(!data_block) {
		fprintf(stderr,"WorldFile::load(): can`t get data block in \"%s\" file\n",file_name);
		delete parser;
		return;
	}
	
	data = data_block;
	
	try {
		while(*data) {
			const char *token = read_token();
			if(!token || !*token) break;
			else if(!strcmp(token,"bsp")) load_bsp();
			else if(!strcmp(token,"light")) load_light();
			//else if(!strcmp(token,"fog")) load_fog();
			//else if(!strcmp(token,"mirror")) load_mirror();
			else if(!strcmp(token,"mesh")) load_mesh();
			//else if(!strcmp(token,"particles")) load_particles();
			else throw(error("unknown token \"%s\"",token));
		}
	}
	catch(const char *msg) {
		fprintf(stderr,"WorldFile::load(): %s in \"%s\" file\n",msg,file_name);
		delete data_block;
		delete parser;
		return;
	}
	
	delete data_block;
	delete parser;
}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

/*
 */
const char *WorldFile::error(const char *error,...) {
	static char buf[1024];
	va_list arg;
	va_start(arg,error);
	vsprintf(buf,error,arg);
	va_end(arg);
	return buf;
}

/*
 */
const char *WorldFile::read_token(char *must) {
	if(!*data) return NULL;
	while(*data && strchr(" \t\n\r",*data)) data++;
	char *token = data;
	char *s = token;
	while(*data && !strchr(" \t\n\r",*data)) *s++ = *data++;
	if(*data) data++;
	*s = '\0';
	if(must) if(strcmp(token,must)) throw("unknown token");
	return token;
}

int WorldFile::read_bool() {
	const char *token = read_token();
	if(token) {
		if(!strcmp(token,"false") || !strcmp(token,"0")) return 0;
		if(!strcmp(token,"true") || !strcmp(token,"1")) return 1;
		throw(error("unknown token \"%s\" in read_bool",token));
		return 1;
	}
	return 0;
}

int WorldFile::read_int() {
	const char *token = read_token();
	if(token) {
		if(!strchr("-01234567890",*token)) throw(error("unknown token \"%s\" in read_int",token));
		return atoi(token);
	}
	return 0;
}

float WorldFile::read_float() {
	const char *token = read_token();
	if(token) {
		if(!strchr("-01234567890.",*token)) throw(error("unknown token \"%s\" in read_float",token));
		return atof(token);
	}
	return 0.0;
}

Vector3 WorldFile::read_vec3() {
	Vector3 ret;
	ret.set(read_float(), 'x');
	ret.set(read_float(), 'y');
	ret.set(read_float(), 'z');
	return ret;
}

Vector4 WorldFile::read_vec4() {
	Vector4 ret;
	ret.set(read_float(), 'x');
	ret.set(read_float(), 'y');
	ret.set(read_float(), 'z');
	ret.set(read_float(), 'w');
	return ret;
}

const char *WorldFile::read_string() {
	if(!*data) return NULL;
	while(*data && strchr(" \t\n\r",*data)) data++;
	char *str = data;
	char *s = str;
	if(*str == '\"') {
		while(*data && !strchr(" \"",*data)) *s++ = *data++;
		if(*data) data++;
		*s = '\0';
		return str + 1;
	}
	while(*data && !strchr(" \t\n\r",*data)) *s++ = *data++;
	if(*data) data++;
	*s = '\0';
	return str;
}

/*****************************************************************************/
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

/*
 */
void WorldFile::load_bsp() 
{
	read_token("{");
	Core::bsp_ = new BSPTree();
	while(1) {
		const char *token = read_token();
		if(!token || !strcmp(token,"}")) break;
		else if(!strcmp(token,"mesh")) Core::bsp_->load(Core::FindFile(read_string()));
		//else if(!strcmp(token,"save")) Core::bsp_->save(read_string());
		else if(!strcmp(token,"material")) {
			const char *name = read_string();
			Core::bsp_->bindMaterial(name,Core::LoadMaterial(read_string()));
		}
		else throw(error("unknown token \"%s\" in bsp block",token));
	}
}

/*
 */
void WorldFile::load_pos(Position &pos,Matrix4 &matrix) {
	read_token("{");
	const char *token = read_token();
	if(!strcmp(token,"spline")) {
		read_token("{");
		char name[1024];
		int close = 0;
		int follow = 0;
		float speed = 1;
		while(1) {
			const char *token = read_token();
			if(!token || !strcmp(token,"}")) break;
			else if(!strcmp(token,"path")) strcpy(name,read_string());
			else if(!strcmp(token,"speed")) speed = read_float();
			else if(!strcmp(token,"close")) close = read_bool();
			else if(!strcmp(token,"follow")) follow = read_bool();
			else throw(error("unknown token \"%s\" in spline block",token));
		}
		Spline *spline = new Spline(Core::FindFile(name),speed,close,follow);
		pos.setSpline(spline);
		matrix = spline->to_matrix(0.0);
		read_token("}");
	} 
	else 
	{
		float m[16];
		m[0] = atof(token);
		m[1] = read_float();
		m[2] = read_float();
		const char *token = read_token();
		if(!strcmp(token,"}")) 
		{
			matrix.setTranslate(Vector3(m[0],m[1],m[2]));
		} 
		else 
		{
			/*m[3] = atof(token);
			m[4] = read_float();
			m[5] = read_float();
			m[6] = read_float();
			const char *token = read_token();
			if(!strcmp(token,"}")) 
			{
			quat rot(m[3],m[4],m[5],m[6]);
			Matrix4 translate;
			translate.setTranslate(Vector3(m[0],m[1],m[2]));
			matrix = translate * rot.to_matrix();
			} 
			else 
			{
			m[7] = atof(token);
			for(int i = 8; i < 16; i++) m[i] = read_float();
			matrix = Matrix4(m);
			read_token("}");
			}*/
			printf("Reading: encountered quaternion\n");
		}
		pos = matrix.multiplyVec3(Vector3(0,0,0));
	}
}

/*
 */
void WorldFile::load_light() {
	read_token("{");
	Position pos(Vector3(0, 0, 0));
	Matrix4 matrix;
	float radius = 0.0;
	Vector4 color(1.0,1.0,1.0,1.0);
	int shadows = 1;
	Material *material = NULL;
	while(1) {
		const char *token = read_token();
		if(!token || !strcmp(token,"}")) break;
		else if(!strcmp(token,"pos")) load_pos(pos,matrix);
		else if(!strcmp(token,"radius")) radius = read_float();
		else if(!strcmp(token,"color")) color = read_vec4();
		else if(!strcmp(token,"shadows")) shadows = read_bool();
		else if(!strcmp(token,"material")) {
			read_string();
			material = Core::LoadMaterial(read_string());
		}
		else throw(error("unknown token \"%s\" in light block",token));
	}
	Light *light = new Light(Vector3(0,0,0),radius,color,shadows);
	if(material) light->bindMaterial("*",material);
	light->pos_ = pos;
	light->pos_.setRadius(radius);
	light->setTransform(matrix);
	Core::AddLight(light);
}

/*
 */
void WorldFile::load_mesh() 
{
	read_token("{");
	OGeometry *mesh = NULL;
	int shadows = 1;
	while(1) {
		const char *token = read_token();
		if(!token || !strcmp(token,"}")) break;
		else if(!strcmp(token,"mesh"))
		{
			mesh = new OGeometry(read_string());
		}
	//	else if(!strcmp(token,"shadows")) shadows = read_bool();
		else if(mesh && !strcmp(token,"material")) {
			const char *name = read_string();
			mesh->bindMaterial(name,Core::LoadMaterial(read_string()));
		}
		else if(mesh && !strcmp(token,"pos")) {
			Matrix4 matrix;
			load_pos(mesh->pos_,matrix);
			mesh->set_transform(matrix);
		}
		/*else if(mesh && !strcmp(token,"rigidbody")) {
		read_token("{");
		Matrix4 matrix;
		float mass = 0;
		float restitution = 0;
		float friction = 0;
		int flag = 0;
		while(1) {
		const char *token = read_token();
		if(!token || !strcmp(token,"}")) break;
		else if(!strcmp(token,"pos")) load_pos(mesh->pos_,matrix);
		else if(!strcmp(token,"mass")) mass = read_float();
		else if(!strcmp(token,"restitution")) friction = read_float();
		else if(!strcmp(token,"friction")) friction = read_float();
		else if(!strcmp(token,"collide")) {
		const char *collide = read_string();
		if(!strcmp(collide,"mesh")) flag |= RigidBody::COLLIDE_MESH;
		else if(!strcmp(collide,"sphere")) flag |= RigidBody::COLLIDE_SPHERE;
		else throw(error("unknown collide \"%s\" in rigidbody block",collide));
		} else if(!strcmp(token,"body")) {
		const char *body = read_string();
		if(!strcmp(body,"box")) flag |= RigidBody::BODY_BOX;
		else if(!strcmp(body,"sphere")) flag |= RigidBody::BODY_SPHERE;
		else if(!strcmp(body,"cylinder")) flag |= RigidBody::BODY_CYLINDER;
		else throw(error("unknown body \"%s\" in rigidbody block",body));
		} else throw(error("unknown token \"%s\" in rigidbody block",token));
		}
		mesh->setRigidBody(new RigidBody(mesh,mass,restitution,friction,flag));
		mesh->set_transform(matrix);
		}*/
		else throw(error("unknown token \"%s\" in mesh block",token));
	}
	if(mesh) {
		mesh->setShadows(shadows);
		Core::AddObject(mesh);
	}
	else fprintf(stderr,"WorldFile::load_mesh(): can`t find mesh\n");
}

/*
*//*
void WorldFile::load_particles() {
read_token("{");
Position pos(Vector3(0, 0, 0));
Matrix4 matrix;
int num = 0;
float speed = 0.0;
float rotation = 0.0;
Vector3 force(0.0,0.0,0.0);
float time = 0.0;
float radius = 0.0;
Vector4 color(1.0,1.0,1.0,1.0);
Material *material = NULL;
while(1) {
const char *token = read_token();
if(!token || !strcmp(token,"}")) break;
else if(!strcmp(token,"pos")) load_pos(pos,matrix);
else if(!strcmp(token,"num")) num = read_int();
else if(!strcmp(token,"speed")) speed = read_float();
else if(!strcmp(token,"rotation")) rotation = read_float();
else if(!strcmp(token,"force")) force = read_vec3();
else if(!strcmp(token,"time")) time = read_float();
else if(!strcmp(token,"radius")) radius = read_float();
else if(!strcmp(token,"color")) color = read_vec4();
else if(!strcmp(token,"material")) {
read_string();
material = Core::LoadMaterial(read_string());
} else throw(error("unknown token \"%s\" in particles block",token));
}
OParticles *particles = new OParticles(new Particles(num,Vector3(0,0,0),speed,rotation,force,time,radius,color));
if(material) particles->bindMaterial("*",material);
particles->pos_ = pos;
particles->set_transform(matrix);
Core::AddObject(particles);
}*/
