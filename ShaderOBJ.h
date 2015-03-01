#ifndef SHADER_OBJ_H
#define SHADER_OBJ_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

class ShaderOBJ
{
private:
	GLuint* gShaderProgram;
	bool compile();

public:
	ShaderOBJ(GLuint* gShaderP);
	ShaderOBJ();
	void Uniforms();
	~ShaderOBJ();
	
	GLint model;
	GLint view;
	GLint proj;
	GLint textureSample;

	void CompileErrorPrint(GLuint* shader);
	void LinkErrorPrint(GLuint* gShaderProgram);
};

#endif