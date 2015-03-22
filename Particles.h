#ifndef PARTICLES_H
#define PATICLES_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
class Particles
{
public:
	Particles();
	~Particles();
	struct Particle
	{
		glm::vec3 pos;
		float xPos;
		glm::vec3 velocity;
		float yPos;
		Particle(){};
		Particle(glm::vec3 pPos, glm::vec3 pVelocity, float pXPos, float pYPos)
		{
			pos = pPos;
			velocity = pVelocity;
			xPos = pXPos;
			yPos = pYPos;
		}
	};
	
	Particle* particlesArray;
	GLuint paticleVBO;
	void Bind();
	void createPaticleData(float hMapStartX, float hMapStartZ);
};

#endif