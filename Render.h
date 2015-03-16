#ifndef RENDER_H
#define RENDER_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "ShaderOBJ.h"
#include "ShaderSMap.h"
#include "ShaderHMap.h"
#include "ShaderBlit.h"
#include "ShaderCompute.h"
#include "ShaderParticle.h"
#include "BlitQuad.h"
#include "obj.h"
#include "SpotLight.h"
#include "ShadowMapFBO.h"
#include "gbuffer.h"
#include "UserInput.h"
#include "HeightMapdata.h"
#include "Vertex.h"
#include "Particles.h"
#include "QuadTree.h"

#define BUFFER_OFFSET(i) ((char *)nullptr + (i))
#define GLM_FORCE_RADIANS

class Render
{
private:
	void setLights();
	BlitQuad* blitQuads;
	QuadTree* QT;
	glm::vec3 lastPos;
	bool insideBorders = false;
public:
	Render(unsigned int* width, unsigned int* height);
	~Render();
	void Init();
	unsigned int* windowWidth;
	unsigned int* windowHeight;

	GLuint gVertexAttribute = 0;

	//Main shaders-------------
	//DeferedPass
	GBuffer* gBuffer;
	GLuint gShaderProgramOBJ = 0;
	ShaderOBJ* shaderOBJ;
	//Blit/Lightning Pass
	GLuint gShaderProgramBlit = 0;
	ShaderBlit* shaderBlit;

	//Side Shaders-------------
	//ShadowmapPass
	GLuint gShaderProgramSMap = 0;
	ShaderSMap* shaderSMap;
	//Heightmap
	GLuint gShaderProgramHMap = 0;
	ShaderHMap* shaderHMap;
	//particles
	GLuint gShaderProgramParticle = 0;
	ShaderParticle* shaderParticle;

	//compute shader for particle program
	GLuint gShaderProgramCompute = 0;
	ShaderCompute* shaderCompute;

	void createPaticleData();
	Particles* particles;

	void GeometryPassInit();
	int GeometryPass(Obj* object);
	void GeometryPassHMap();
	void GeometryPassParticle();
	void ShadowMapPassInit();
	void ShadowMapPass(Obj* object);
	void LightPass();

	//main camera matrixes
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0, 0, -2), glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
	glm::mat4 projMatrix = glm::perspective(3.14f*0.45f, 640.f / 480.0f, 0.1f, 1000.0f);

	//game objects
	bool OBJinit = false;
	Obj* obj = 0;
	Obj* obj2 = 0;

	//Light
	SpotLight* spotLights;
	int nrSpotLights;
	ShadowMapFBO* shadowMap;
	int nrSpotLightsShadow;

	HeightMapdata* heightMap;

	//user input
	UserInput* in;
};

#endif