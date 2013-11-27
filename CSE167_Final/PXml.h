#pragma once
#include <xml/pugixml.hpp>
#include <vector>
#include <string>

/* stands for portable xml */
class PXml
{
public:
	PXml(void);
	~PXml(void);

	int load(const char* name);
	void clear();
	const std::string& getArg(const char *name) const;
	int getBoolArg(const char *name) const;
	int getBoolData() const;
	const std::string& getData() const;
	float getFloatArg(const char *name) const;
	int getIntArg(const char *name) const;
	int getIntArrayData(int * 	dest, int 	size ) const; //terrain, water
	int getIntData() const;

	int getNumChilds() const;
	int getStringArrayArg(const char *name, std::vector<std::string> &dest) const;
	int isArg(const char *name) const;

private:
	std::string name;
	struct Argument {
		std::string name;				// argument name
		std::string value;				// argument value
	};
	std::vector<Argument,short> args;	// node args
};

