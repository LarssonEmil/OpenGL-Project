#include "ShaderSMap.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

ShaderSMap::ShaderSMap(GLuint* gShaderP)
{
	gShaderProgram = gShaderP;
	compile();

	model = glGetUniformLocation(*gShaderProgram, "World");
	view = glGetUniformLocation(*gShaderProgram, "View");
	proj = glGetUniformLocation(*gShaderProgram, "Projection");
	normal = glGetUniformLocation(*gShaderProgram, "NormalMatrix");
}

bool ShaderSMap::compile()
{
	const char* vertex_shader = R"(
	#version 410
	layout (location = 0) in vec3 Position; 
	layout (location = 1) in vec2 UV;                                             

	uniform mat4 World;
	uniform mat4 View;
	uniform mat4 Projection;                                                                                                                      

	void main()
	{       
		gl_Position = (Projection * View * World * vec4(Position, 1.0));
	}
)";

	GLint success = 0;

	//create vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, nullptr);
	glCompileShader(vs);
	CompileErrorPrint(&vs);

	//link shader program (connect vs and ps)
	*gShaderProgram = glCreateProgram();
	glAttachShader(*gShaderProgram, vs);
	glLinkProgram(*gShaderProgram);
	LinkErrorPrint(gShaderProgram);

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
	return true;
}

void ShaderSMap::CompileErrorPrint(GLuint* shader)
{
	GLint success = 0;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(*shader, maxLength, &maxLength, &errorLog[0]);

		std::fstream myfile;
		myfile.open("errorCheck.txt", std::fstream::out);
		for (int i = 0; i < maxLength; i++)
		{
			myfile << errorLog[i];
		}
		myfile.close();

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(*shader); // Don't leak the shader.
		throw;
	}
}

void ShaderSMap::LinkErrorPrint(GLuint* shaderProgram)
{
	GLint success = 0;
	glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(*shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(*shaderProgram, maxLength, &maxLength, &errorLog[0]);

		std::fstream myfile;
		myfile.open("errorCheck.txt", std::fstream::out);
		for (int i = 0; i < maxLength; i++)
		{
			myfile << errorLog[i];
		}
		myfile.close();

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteProgram(*shaderProgram); // Don't leak the shader.

		if (success == GL_FALSE)
			throw;
	}
}