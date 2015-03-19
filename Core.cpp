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
		rend->obj = new Obj("floor", objectCounter++, 5.0f);
		rend->obj2 = new Obj("boat", objectCounter++, 0.01f);
		rend->obj->translate(128.0f, 9.0f, 128.0f);
		rend->obj->scaleUniform(30.0f);
		rend->obj2->translate(60.0f, 10.0f, 70.0f);
		rend->obj2->scaleUniform(-0.95f);

		MakeLights();
		wave = 0.0f;
	}
	//wave += 0.01f;
	//UpdateLights(wave);

	rend->GeometryPassInit();
	rend->GeometryPass(rend->obj2);
	rend->GeometryPass(rend->obj);
	rend->GeometryPassHMap();
	rend->GeometryPassParticle();
	
	rend->obj2->rotate(0, 0.1f, 0);
	if (rend->nrSpotLightsShadow > 0)
	{
		rend->ShadowMapPassInit(); 
		//rend->ShadowMapPass(rend->obj); flat objects dont cast shadows
		rend->ShadowMapPass(rend->obj2);
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
	rend->nrSpotLights = 2; //including lights that cast shadows
	rend->nrSpotLightsShadow = 1;

	rend->shadowMap = new ShadowMapFBO();
	rend->shadowMap->Init(WINDOW_WITH, WINDOW_HEIGHT);

	rend->spotLights = new SpotLight[rend->nrSpotLights];

	rend->spotLights[0].Color = vec3(1.0f, 1.0f, 1.0f); 
	rend->spotLights[0].Position  = vec3(0.0f, 125.0f, 70.0f);
	rend->spotLights[0].Direction = normalize(vec3(0.4f, -0.6f, 0.0f));
	rend->spotLights[0].DiffuseIntensity = 0.50f;
	rend->spotLights[0].AmbientIntensity = 0.0f;
	rend->spotLights[0].Cutoff = 0.9f;

	rend->spotLights[1].Color = vec3(1.0f, 1.0f, 1.0f);
	rend->spotLights[1].Position = vec3(0.0f, 125.0f, 70.0f);
	rend->spotLights[1].Direction = normalize(vec3(0.4f, -0.6f, 0.0f));
	rend->spotLights[1].DiffuseIntensity = 0.4f;
	rend->spotLights[1].AmbientIntensity = 0.4f;
	rend->spotLights[1].Cutoff = 0.01f;

	//int count = 1;
	//int max = 4;
	//int maxii = max - 1;
	//for (int x = 0; x < max; x++)
	//{
	//	for (int y = 0; y < max; y++)
	//	{
	//		if (x == 0 || x == maxii || y == 0 || y == maxii)
	//		{
	//			rend->spotLights[count].Color = vec3(	0.0f + float(x) / maxii, 
	//													1.0f - (float(y) / float(maxii) + float(x) / float(maxii)) / 2,
	//													1.0f - float(y) / maxii);
	//			rend->spotLights[count].Position = vec3(((float(x) / float(maxii)) - 0.5f) * 10, -0.5f, ((float(y) / float(maxii)) - 0.5f) * 10);
	//			count++;
	//		}
	//	}
	//}
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

