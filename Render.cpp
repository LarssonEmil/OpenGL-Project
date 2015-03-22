#include "Render.h"
#include "bth_image.h"
#include <fstream>
#include <vector>

Render::Render(unsigned int* width, unsigned int* height)
{
	windowWidth = width;
	windowHeight = height;
}

Render::~Render()
{
	delete [] blitQuads;
	delete QT;
	delete gBuffer;
	delete shaderOBJ;
	delete shaderBlit;
	delete shaderSMap;
	delete shaderHMap;
	delete shaderParticle;
	delete shaderCompute;
	delete particles;
	if (obj)
		delete obj;
	if (obj2)
		delete obj2;
	delete heightMap;
	delete in;
}

void Render::Init()
{
	shaderOBJ = new ShaderOBJ(&gShaderProgramOBJ);
	shaderSMap = new ShaderSMap(&gShaderProgramSMap);
	shaderBlit = new ShaderBlit(&gShaderProgramBlit);
	shaderParticle = new ShaderParticle(&gShaderProgramParticle);
	shaderCompute = new ShaderCompute(&gShaderProgramCompute);

	gBuffer = new GBuffer();
	gBuffer->Init(*windowWidth, *windowHeight);
	//make blitquads
	blitQuads = new BlitQuad[6];
	blitQuads[0].Init(&gShaderProgramBlit, vec2(-1,-1), vec2(-0.6, -0.6));
	blitQuads[1].Init(&gShaderProgramBlit, vec2(-0.6, -1), vec2(-0.2, -0.6));
	blitQuads[2].Init(&gShaderProgramBlit, vec2(-0.2, -1), vec2(0.2, -0.6));
	blitQuads[3].Init(&gShaderProgramBlit, vec2(0.2, -1), vec2(0.6, -0.6));
	blitQuads[4].Init(&gShaderProgramBlit, vec2(0.6, -1), vec2(1, -0.6));
	blitQuads[5].Init(&gShaderProgramBlit, vec2(-1, -1), vec2(1, 1));

	shaderHMap = new ShaderHMap(&gShaderProgramHMap);
	heightMap = new HeightMapdata();
	heightMap->Init();
	particles = new Particles();
	particles->createPaticleData(heightMap->getStartingX(), heightMap->getStartingZ());
	QT = new QuadTree(heightMap->subIndexBuffers, 256);
}

void Render::GeometryPassInit()
{
	glUseProgram(gShaderProgramOBJ);
	gBuffer->BindForWriting();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int Render::GeometryPass(Obj* object)
{
	object->Bind();
	glProgramUniformMatrix4fv(gShaderProgramOBJ, shaderOBJ->model, 1, false, &(object->worldMatrix[0][0]));
	glProgramUniformMatrix4fv(gShaderProgramOBJ, shaderOBJ->view, 1, false, &viewMatrix[0][0]);
	glProgramUniformMatrix4fv(gShaderProgramOBJ, shaderOBJ->proj, 1, false, &projMatrix[0][0]);
	glProgramUniform1i(gShaderProgramOBJ, shaderOBJ->textureSample, 6 + object->id);

	glDrawElements(GL_TRIANGLES, object->faceCount*3, GL_UNSIGNED_SHORT, 0);

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
	return 1;
}

void Render::GeometryPassHMap()
{
	glUseProgram(gShaderProgramHMap);
	bool insideBorders = false;
	if (lastPos != *in->GetPos() || lastDir != in->getToTarget())
	{
		//glm::mat4 wombocombomatrix = projMatrix * viewMatrix;
		//QT->ExtractPlanes(&wombocombomatrix, true);

		//if(lastPos != *in->GetPos())
		//	insideBorders = heightMap->terrainCollison(*in->GetPos());
	}

	//glMemoryBarrier(GL_ALL_BARRIER_BITS); //<--- ????

	heightMap->Bind(&gShaderProgramHMap, shaderHMap);

	glProgramUniformMatrix4fv(gShaderProgramHMap, shaderHMap->ViewMatrix, 1, false, &viewMatrix[0][0]);
	glProgramUniform2fv(gShaderProgramHMap, shaderHMap->cameraUV, 1, &heightMap->cameraUV[0]);
	glProgramUniformMatrix4fv(gShaderProgramHMap, shaderHMap->ProjectionMatrix, 1, false, &projMatrix[0][0]);
	glProgramUniform1f(gShaderProgramHMap, shaderHMap->mat1Scale, heightMap->mat1Scale);
	glProgramUniform1f(gShaderProgramHMap, shaderHMap->mat2Scale, heightMap->mat2Scale);

	glBindVertexArray(heightMap->gHeightMapAttribute);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, heightMap->ssbo);
	glBindBuffer(GL_ARRAY_BUFFER, heightMap->gHeightMapBuffer);

	//traverse tree and draw
	QT->Draw(QT->root, 5, &viewMatrix);

	if (insideBorders)
	{
		float* tmpY = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 1, GL_MAP_READ_BIT);
		if (tmpY != nullptr)
			in->moveCameraDirection(tmpY);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	lastPos = *in->GetPos();
	lastDir = in->getToTarget();
}

void Render::GeometryPassParticle()
{
	//compute shader
	glUseProgram(gShaderProgramCompute);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, heightMap->getTexture());
	//glProgramUniform1i(gShaderProgramCompute, shaderCompute->heightMapSampler, 0);
	glProgramUniform1f(gShaderProgramCompute, shaderCompute->gridWidthU, heightMap->getGridWidth());
	glProgramUniform1f(gShaderProgramCompute, shaderCompute->gridHeightV, heightMap->getGridHeight());

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, particles->paticleVBO);
	glDispatchCompute(20, 1, 1);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif

	//Shaders for the particle system
	glUseProgram(gShaderProgramParticle);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, particles->paticleVBO);
	glProgramUniformMatrix4fv(gShaderProgramParticle, shaderParticle->ViewMatrix, 1, false, &viewMatrix[0][0]);
	glProgramUniformMatrix4fv(gShaderProgramParticle, shaderParticle->ProjectionMatrix, 1, false, &projMatrix[0][0]);

	glDrawArrays(GL_POINTS, 0, 500);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}

void Render::ShadowMapPassInit()
{
	glUseProgram(gShaderProgramSMap);
	shadowMap->BindForWriting(); //clears
}

void Render::ShadowMapPass(Obj* object)
{
	//mat4 save = viewMatrix;
	object->Bind();
	for (int n = 0; n < nrSpotLightsShadow; n++)
	{
		viewMatrix = glm::lookAt(spotLights[n].Position, spotLights[n].Position + spotLights[n].Direction, vec3(0, 1, 0));
		glProgramUniformMatrix4fv(gShaderProgramSMap, shaderSMap->model, 1, false, &(object->worldMatrix[0][0]));
		glProgramUniformMatrix4fv(gShaderProgramSMap, shaderSMap->view, 1, false, &viewMatrix[0][0]);
		glProgramUniformMatrix4fv(gShaderProgramSMap, shaderSMap->proj, 1, false, &projMatrix[0][0]);
		glDrawElements(GL_TRIANGLES, object->faceCount * 3, GL_UNSIGNED_SHORT, 0);
	}
	//viewMatrix = save;
}

void Render::LightPass()
{
	glUseProgram(gShaderProgramBlit);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	gBuffer->BindForReading();

	glProgramUniform1i(gShaderProgramBlit, shaderBlit->Position, 0);
	glProgramUniform1i(gShaderProgramBlit, shaderBlit->Diffuse, 1);
	glProgramUniform1i(gShaderProgramBlit, shaderBlit->Normal, 2);
	glProgramUniform1i(gShaderProgramBlit, shaderBlit->UVcord, 3);
	glProgramUniform1i(gShaderProgramBlit, shaderBlit->Depth, 4);

	//final image
	//send all lights
	glProgramUniform3fv(gShaderProgramBlit, shaderBlit->eyepos, 1, &(*in->GetPos())[0]);
	glProgramUniform1i(gShaderProgramBlit, shaderBlit->NumSpotLights, nrSpotLights);
	glProgramUniform1i(gShaderProgramBlit, shaderBlit->NumSpotLightsShadow, nrSpotLightsShadow);

	if (nrSpotLightsShadow > 0)
	{
		shadowMap->BindForReading(5);
		glProgramUniform1i(gShaderProgramBlit, shaderBlit->ShadowMaps, 5);
		mat4 cameraview = glm::lookAt(spotLights[0].Position, spotLights[0].Position + spotLights[0].Direction, vec3(0, 1, 0));
		glProgramUniformMatrix4fv(gShaderProgramBlit, shaderBlit->ViewMatrixSM, 1, false, &cameraview[0][0]);
		glProgramUniformMatrix4fv(gShaderProgramBlit, shaderBlit->ProjectionMatrixSM, 1, false, &projMatrix[0][0]);
	}

	glBindBuffer(GL_UNIFORM_BUFFER, shaderBlit->lightBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(SpotLight) * nrSpotLights, spotLights, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, shaderBlit->bindingPoint, shaderBlit->lightBuffer);

	blitQuads[5].BindVertData();
	glProgramUniform1i(gShaderProgramBlit, shaderBlit->Use, 5);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//each blit
	for (int n = 0; n < 5; n++)
	{
		blitQuads[n].BindVertData();
		glProgramUniform1i(gShaderProgramBlit, shaderBlit->Use, n);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}
