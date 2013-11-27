#include "World.h"


World::World(void)
{
}


World::~World(void)
{
}

void World::ClearWorld()
{
	isLoaded_ = false;
	//TODO: lots of works
}

void World::LoadWorld(const char *filename)
{
	name_ = filename;
	
	//load xml
	//parse xml -> material manager -> library
}