#pragma once
#include <string>
class World
{
public:
	World(void);
	~World(void);

	void LoadWorld(const char *filename);
	void LoadWorldCommand(int argc, char **argv);

	void ClearWorld();

	inline bool isLoaded() const
	{
		return isLoaded_;
	}

private:
	bool isLoaded_;
	std::string name_;
};

