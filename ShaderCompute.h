#ifndef SHADER_COMPUTE_H
#define SHADER_COMPUTE_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <vector>


class ShaderCompute
{
private:
	GLuint* gShaderProgram;
	bool compile();

public:
	ShaderCompute(GLuint* gShaderP);
	ShaderCompute();
	~ShaderCompute();

	GLint ModelMatrix;
	GLint ViewMatrix;
	GLint ProjectionMatrix;
	GLint heightMapSampler;
	GLint gridWidthU;
	GLint gridHeightV;

	void CompileErrorPrint(GLuint* shader);
	void LinkErrorPrint(GLuint* gShaderProgram);
};

#endif