#include "Particles.h"

using namespace glm;
#define MAX_PARTICLES 200

Particles::Particles()
{

}

void Particles::createPaticleData(float hMapStartX, float hMapStartZ)  //Second shader storage buffer, stores particle data.
{
	vec3 velocity = vec3(-0.3f, 1.5f, 0.0f);
	particlesArray = new Particles::Particle[MAX_PARTICLES];
	for (int i = 0; i < MAX_PARTICLES; i++)
	{
		vec3 rPos = vec3(rand() % 258 + hMapStartX, rand() % 40 + 40, rand() % 258 + hMapStartZ);
		particlesArray[i] = Particle(rPos, velocity, rPos.x, rPos.y);
	}

	glGenBuffers(1, &paticleVBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, paticleVBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Particles::Particle)*MAX_PARTICLES, &particlesArray[0], GL_DYNAMIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, paticleVBO);

	delete [] particlesArray;
#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}