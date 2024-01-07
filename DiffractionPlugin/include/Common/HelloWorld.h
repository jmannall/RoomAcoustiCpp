/*
*
*  \HelloWorld Class
*
*/

#ifndef HelloWorld_h
#define HelloWorld_h

#include <string>

class HelloWorld
{
public:
	HelloWorld() : mString("Hello World!") {}
	~HelloWorld() {}

	std::string GetString() { return mString; }
	std::string mString;
};

#endif