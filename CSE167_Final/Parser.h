#pragma once
#include <map>
class Parser 
{
public:

	Parser(const char *name);
	~Parser();

	char *get(const char *name);

	static int read_string(const char *str,char *dest);
	static float expression(const char *str,const char *variable = nullptr,float value = 0.0);
	static const char *interpret(const char *src);

protected:

	static int  priority(char op);

	static int read_token(const char *src,char *dest);
	static int interpret_eq(const char *src);
	static int interpret_if(const char *src,char **dest);
	static int interpret_for(const char *src,char **dest);
	static int interpret_main(const char *src,char **dest);

	char *data;

	struct Block {
		char *pointer;
		int use;
	};

	std::map<std::string, Block> blocks;

	static float variables[26];
};
