#ifndef SHADER_HMAP_H
#define SHADER_HMAP_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

class ShaderHMap
{
private:
	GLuint* gShaderProgram;
	bool compile();

public:
	ShaderHMap(GLuint* gShaderP);
	ShaderHMap();
	~ShaderHMap();

	GLint ViewMatrix;
	GLint ProjectionMatrix;
	GLint heightMapSampler;
	GLint grassSampler;
	GLint roadSampler;
	GLint blendMapSampler;
	GLint cameraUV;
	GLint mat1Scale;
	GLint mat2Scale;

	void CompileErrorPrint(GLuint* shader);
	void LinkErrorPrint(GLuint* gShaderProgram);
};

#endif