#ifndef SHADER_SMAP_H
#define SHADER_SMAP_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "SpotLight.h"

class ShaderSMap
{
private:
	GLuint* gShaderProgram;
	bool compile();

public:
	ShaderSMap(GLuint* gShaderP);
	ShaderSMap();
	void Uniforms();
	~ShaderSMap();

	GLint model;
	GLint view;
	GLint proj;
	GLint normal; //if transposed

	void CompileErrorPrint(GLuint* shader);
	void LinkErrorPrint(GLuint* gShaderProgram);
};

#endif