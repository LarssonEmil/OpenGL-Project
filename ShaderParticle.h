#ifndef SHADER_PARTICLE_H
#define SHADER_PARTICLE_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>


class ShaderParticle
{
private:
	GLuint* gShaderProgram;
	bool compile();

public:
	ShaderParticle(GLuint* gShaderP);
	ShaderParticle();
	~ShaderParticle();

	GLint ModelMatrix;
	GLint ViewMatrix;
	GLint ProjectionMatrix;

	void CompileErrorPrint(GLuint* shader);
	void LinkErrorPrint(GLuint* gShaderProgram);
};

#endif