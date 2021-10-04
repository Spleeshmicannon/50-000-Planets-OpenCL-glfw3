#pragma once
// OpenGL header
#include <GL/glew.h>

// standard headers
#include <string>
#include <iostream>
#include <fstream>

namespace
{
	__forceinline std::string readFile(const std::string filename)
	{
		std::string data;
		std::getline(std::ifstream(filename), data, '\0');

		if (data.size() == 0)
		{
			std::cerr << "No file found: " << filename << "\t\n";
		}

		return data;
	}
}