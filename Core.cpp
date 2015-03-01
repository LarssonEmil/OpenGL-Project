#include "Core.h"

Core::Core()
{
	rend = new Render(&WINDOW_WITH,&WINDOW_HEIGHT);
	//need better way to set camera start
	rend->in = new UserInput(&rend->viewMatrix, glm::vec3(0, 0, -4), glm::vec3(0, 0, 3), glm::vec3(0, 1, 0));
#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}

int Core::update()
{
	//update game logic..
	//init
	if (rend->OBJinit == false)
	{
		rend->OBJinit = true;
		rend->obj = new Obj("floor", objectCounter++);
		rend->obj2 = new Obj("cubeman", objectCounter++);
		MakeLights();
		wave = 0.0f;
	}
	wave += 0.01f;
	UpdateLights(wave);

	rend->GeometryPassInit();
	rend->GeometryPass(rend->obj);
	rend->GeometryPass(rend->obj2);
	rend->GeometryPassHMap();
	rend->GeometryPassParticle();

	//rotate light
	float rottemp = 0.001f;
	glm::mat3 rot = glm::mat3(cos(rottemp), 0.0f, -sin(rottemp),
		0.0f, 1.0f, 0.0f,
		sin(rottemp), 0.0f, cos(rottemp));
	rend->spotLights[0].Position = rend->spotLights[0].Position* rot;
	rend->spotLights[0].Direction = rend->spotLights[0].Direction* rot;
	
	rend->obj2->rotate(0, 0.1f, 0, false);

	if (rend->nrSpotLightsShadow > 0)
	{
		rend->ShadowMapPassInit();
		glCullFace(GL_FRONT);
		rend->ShadowMapPass(rend->obj);
		rend->ShadowMapPass(rend->obj2);
		glCullFace(GL_BACK);
	}

	rend->LightPass();

#ifdef _DEBUG
	{GLenum err = glGetError();if (err)
			int x = 0;}
#endif
	return 1;
}

void Core::MakeLights()
{
	rend->nrSpotLights = 14; //including lights that cast shadows
	rend->nrSpotLightsShadow = 1;

	//Shadowmap lights INIT
	//rend->shadowMap = new ShadowMapFBO[rend->nrSpotLightsShadow];
	//for (int n = 0; n < rend->nrSpotLightsShadow; n++)
	//	rend->shadowMap[0].Init(WINDOW_WITH, WINDOW_HEIGHT);
	rend->shadowMap = new ShadowMapFBO();
	rend->shadowMap->Init(WINDOW_WITH, WINDOW_HEIGHT);

	//regular lights INIT
	rend->spotLights = new SpotLight[rend->nrSpotLights];

	rend->spotLights[0].Color = vec3(1.0f, 1.0f, 1.0f); 
	rend->spotLights[0].Position = vec3(5.0f, 2.0f, 5.0f);
	rend->spotLights[0].Direction = normalize(vec3(-2.0f, -1.0f, -2.0f));
	rend->spotLights[0].DiffuseIntensity = 1.00f;
	rend->spotLights[0].AmbientIntensity = 0.2f;
	rend->spotLights[0].Cutoff = 0.9f;

	rend->spotLights[13].Color = vec3(1.0f, 1.0f, 1.0f);
	rend->spotLights[13].Position = vec3(0.0f, 50.0f, 0.0f);
	rend->spotLights[13].Direction = normalize(vec3(0.3f, -0.3f, 0.3f));
	rend->spotLights[13].DiffuseIntensity = 1.00f;
	rend->spotLights[13].AmbientIntensity = 0.0f;
	rend->spotLights[13].Cutoff = 0.1f;

	int count = 1;
	int max = 4;
	int maxii = max - 1;
	for (int x = 0; x < max; x++)
	{
		for (int y = 0; y < max; y++)
		{
			if (x == 0 || x == maxii || y == 0 || y == maxii)
			{
				rend->spotLights[count].Color = vec3(	0.0f + float(x) / maxii, 
														1.0f - (float(y) / float(maxii) + float(x) / float(maxii)) / 2,
														1.0f - float(y) / maxii);
				rend->spotLights[count].Position = vec3(((float(x) / float(maxii)) - 0.5f) * 10, -0.5f, ((float(y) / float(maxii)) - 0.5f) * 10);
				count++;
			}
		}
	}
}

void Core::UpdateLights(float wave)
{
	int count = 1;
	int max = 4;
	int maxii = max - 1;
	for (int x = 0; x < max; x++)
	{
		for (int y = 0; y < max; y++)
		{
			if (x == 0 || x == maxii || y == 0 || y == maxii)
			{
				rend->spotLights[count].Position = vec3((((float(x) / float(maxii)) - 0.5f) * 10) * ((cos(float(x) / float(maxii) + wave) + 1.0f) / 2.0f), -0.5f, (((float(y) / float(maxii)) - 0.5f) * 10)  * ((cos(float(y) / float(maxii) + wave) + 1.0f) / 2.0f));
				count++;
			}
		}
	}
}

