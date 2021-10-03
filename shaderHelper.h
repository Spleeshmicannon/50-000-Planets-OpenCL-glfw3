#pragma once
// OpenGL header
#include <GL/glew.h>

// standard headers
#include <string>
#include <iostream>
#include <fstream>

namespace
{
	enum class shaderType
	{
		vertex,
		fragment
	};

	template<shaderType st = shaderType::vertex>
	int compileShader(const std::string& source, GLuint type);


	template<> int compileShader<shaderType::vertex>(const std::string& source, GLuint type)
	{
		GLuint id = glCreateShader(GL_VERTEX_SHADER);
		const char* _source = source.c_str();
		glShaderSource(id, 1, &_source, nullptr);
		glCompileShader(id);

		int success;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			char* message = (char*)alloca(length * sizeof(char));
			glGetShaderInfoLog(id, length, &length, message);
			std::cerr << "Failed to compile vertex shader " << message << std::endl;
			glDeleteShader(id);
			return 0;
		}

		return id;
	}

	template<> int compileShader<shaderType::fragment>(const std::string& source, GLuint type)
	{
		GLuint id = glCreateShader(GL_VERTEX_SHADER);
		const char* _source = source.c_str();
		glShaderSource(id, 1, &_source, nullptr);
		glCompileShader(id);

		int success;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			int length;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
			char* message = (char*)alloca(length * sizeof(char));
			glGetShaderInfoLog(id, length, &length, message);
			std::cerr << "Failed to compile fragment shader: " << message << std::endl;
			glDeleteShader(id);
			return 0;
		}

		return id;
	}

	GLuint createShader(const std::string vertexShaderSource, const std::string fragmentShaderSource)
	{
		GLuint program = glCreateProgram();

		GLuint vertexShader = compileShader<shaderType::vertex>(vertexShaderSource, GL_VERTEX_SHADER);
		GLuint fragmentShader = compileShader<shaderType::fragment>(fragmentShaderSource, GL_FRAGMENT_SHADER);

		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		glLinkProgram(program);
		glValidateProgram(program);

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return program;
	}

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

	__forceinline void initOpenGL()
	{
		glewExperimental = true;
		const GLint GlewInitResult = glewInit();
		if (GLEW_OK != GlewInitResult)
		{
			std::cerr << "GLEW ERROR: " << glewGetErrorString(GlewInitResult) << "\t\n";
			exit(EXIT_FAILURE);
		}
	}
}