#pragma once

#include <string>

class HelloWorld
{
public:
	HelloWorld() : mString("Hello World!") {}
	~HelloWorld() {}

	std::string GetString() { return mString; }
	std::string mString;
};