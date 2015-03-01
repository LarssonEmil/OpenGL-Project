#ifndef SHADER_BLIT_H
#define SHADER_BLIT_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "SpotLight.h"

class ShaderBlit
{
private:
	GLuint* gShaderProgram;
	bool compile();

public:
	ShaderBlit(GLuint* gShaderP);
	ShaderBlit();
	void Uniforms();
	~ShaderBlit();

	GLint Use;
	GLint Position;
	GLint Diffuse;
	GLint Normal;
	GLint UVcord;
	GLint Depth;

	GLint NumSpotLights;
	GLint NumSpotLightsShadow;
	GLint ShadowMaps;
	GLint ProjectionMatrixSM;
	GLint ViewMatrixSM;
	GLint eyepos;

	GLuint lightBuffer, lightBlockUniformLoc;
	GLuint bindingPoint = 1;

	void CompileErrorPrint(GLuint* shader);
	void LinkErrorPrint(GLuint* gShaderProgram);
};

#endif