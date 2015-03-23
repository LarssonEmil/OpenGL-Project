#include "ShaderBlit.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

ShaderBlit::ShaderBlit(GLuint* gShaderP)
{
	gShaderProgram = gShaderP;
	compile();

	Use = glGetUniformLocation(*gShaderProgram, "Use");

	Position = glGetUniformLocation(*gShaderProgram, "Position");
	Diffuse = glGetUniformLocation(*gShaderProgram, "Diffuse");
	Normal = glGetUniformLocation(*gShaderProgram, "Normal");
	UVcord = glGetUniformLocation(*gShaderProgram, "UVcord");
	Depth = glGetUniformLocation(*gShaderProgram, "Depth");
	//light
	NumSpotLights = glGetUniformLocation(*gShaderProgram, "NumSpotLights");
	
	NumSpotLightsShadow = glGetUniformLocation(*gShaderProgram, "NumSpotLightsShadow");
	ShadowMaps = glGetUniformLocation(*gShaderProgram, "ShadowMaps");
	ProjectionMatrixSM = glGetUniformLocation(*gShaderProgram, "ProjectionMatrixSM");
	ViewMatrixSM = glGetUniformLocation(*gShaderProgram, "ViewMatrixSM");

	eyepos = glGetUniformLocation(*gShaderProgram, "eyepos");
	lightBlockUniformLoc = glGetUniformBlockIndex(*gShaderProgram, "Light");
	glUniformBlockBinding(*gShaderProgram, lightBlockUniformLoc, bindingPoint);
	glGenBuffers(1, &lightBuffer);


#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}

ShaderBlit::~ShaderBlit()
{
	glDeleteBuffers(1, &lightBuffer);
}

bool ShaderBlit::compile()
{
	const char* vertex_shader = R"(
	#version 410
	layout (location = 0) in vec3 Position;                                             
	layout (location = 1) in vec2 UV;                                             
                                                                                                       
	layout (location = 0) out vec2 UV0;                                                                                                                                 

	void main()
	{       
		gl_Position		= vec4(Position, 1);
		UV0				= UV;    
	}
)";

	const char* fragment_shader = R"(
	#version 410                                                                 
	layout (location = 0) in vec2 UV;                                                                                                                                    

	uniform int Use;
	
	uniform sampler2D Position;
	uniform sampler2D Diffuse;
	uniform sampler2D Normal;
	uniform sampler2D UVcord;	
	uniform sampler2D Depth;
		
	vec4 Position0;
	vec4 Diffuse0;
	vec4 Normal0;
	vec4 Depth0;
	
	struct SpotLight
	{
		vec3 Color;
		float DiffuseIntensity;
		vec3 Position;
		float AmbientIntensity;
		vec3 Direction;
		float Cutoff;
		float Constant;
		float Linear;
		float Exp;
		float padd;
	};

	layout (std140) uniform Light
	{ 
		SpotLight lights[10];
	};
	
	uniform int NumSpotLights;
	uniform int NumSpotLightsShadow; 
	uniform vec3 eyepos;

	uniform sampler2D ShadowMaps;
	uniform mat4 ProjectionMatrixSM;
	uniform mat4 ViewMatrixSM;

	float gSpecularPower = 20;
	float gMatSpecularIntensity = 0.4;

	out vec4 fragment_color;
						
	vec4 CalcLightInternal(SpotLight l, vec3 LightDirection, vec3 Normal)                   
	{                                                                                           
		vec4 AmbientColor = vec4(l.Color, 1.0f) * l.AmbientIntensity;                   
		float DiffuseFactor = dot(Normal, -LightDirection);                                     
                                                                                            
		vec4 DiffuseColor  = vec4(0, 0, 0, 0);                                            
		vec4 SpecularColor = vec4(0, 0, 0, 0);                                                  
                                                                                            
		if (DiffuseFactor > 0) 
		{                                                                
			DiffuseColor = vec4(l.Color, 1.0f) * l.DiffuseIntensity * DiffuseFactor;    
                                                                                            
			vec3 VertexToEye = normalize(eyepos - Position0.xyz);                             
			vec3 LightReflect = normalize(reflect(LightDirection, Normal));                     
			float SpecularFactor = dot(VertexToEye, LightReflect);                              
			SpecularFactor = pow(SpecularFactor, gSpecularPower);                               
			if (SpecularFactor > 0) 
			{                                                           
				SpecularColor = vec4(l.Color, 1.0f) * gMatSpecularIntensity * SpecularFactor;                         
			}                                                                                   
		}                                                                                                                                                                     
		return (AmbientColor + DiffuseColor + SpecularColor);                                   
	}               
	
	vec4 CalcPointLight(SpotLight l, vec3 Normal)
	{
		vec3 LightDirection = Position0.xyz - l.Position;
		float Distance = length(LightDirection);
		LightDirection = normalize(LightDirection);    
	
		return (CalcLightInternal(l, LightDirection, Normal)); 
		//vec4 Color = vec4(CalcLightInternal(l, LightDirection, Normal)); 
		//float Attenuation =  l.Constant + l.Linear * Distance + l.Exp * Distance * Distance; 
		//return Color / Attenuation;                           
	}                                                                                           
                                                                                            
	vec4 CalcSpotLight(SpotLight l, vec3 Normal)                                                
	{                                                                                           
		vec3 LightToPixel = normalize(Position0.xyz - l.Position);                             
		float SpotFactor = dot(LightToPixel, l.Direction);                                      
                                                                                            
		if (SpotFactor > l.Cutoff) {                                                            
			vec4 Color = CalcPointLight(l, Normal);                             
			return Color * (1.0 - (1.0 - SpotFactor) * 1.0/(1.0 - l.Cutoff));                   
		}                                                                                       
		else {                                                                                  
			return vec4(0,0,0,0);                                                               
		}     
	}  		

	float CalcShadowFactor()
	{
		vec4 LightSpacePos = Position0; //pixel world pos
		LightSpacePos = ProjectionMatrixSM * ViewMatrixSM * LightSpacePos;
		vec3 ProjCoords = LightSpacePos.xyz / LightSpacePos.w; //Normalized device coordinates
		vec2 UVCoords;
		UVCoords.x = 0.5 * ProjCoords.x + 0.5;
		UVCoords.y = 0.5 * ProjCoords.y + 0.5;
		float z = 0.5 * ProjCoords.z + 0.5;
		float Depth = texture(ShadowMaps, UVCoords).x;
		if (Depth < (z + 0.00005))
			return 0.01;
		else
			return 1.0;
	}           

	void main()									
	{			
		if(Use == 0)
		{
			fragment_color = texture(Position, UV) * 0.1;
		}
		else if(Use == 1)	 
			fragment_color = texture(Diffuse, vec2(UV.x, UV.y));
		else if(Use == 2)	 
			fragment_color = texture(Normal, vec2(UV.x, UV.y));
		else if(Use == 3)	 
			fragment_color = texture(UVcord, vec2(UV.x, UV.y));
		else if(Use == 4)
		{
			float Depth = texture(Depth, vec2(UV.x, UV.y)).x;
			Depth = 1.0 - (1.0 - Depth) * 25.0; 
			fragment_color = vec4(Depth);
		}
		else if(Use == 5)
		{
			vec4 fogColor = vec4(0.8,0.8,0.9,0.0);
			vec3 dist = eyepos - (texture(Position, UV)).xyz; //world pos
			float len = length(dist);
			float maxdist = 300.0;
			
			fragment_color = vec4(0,0,0,0);
			Diffuse0 = texture(Diffuse, vec2(UV.x, UV.y));

			//if render nothing
			if(Diffuse0.x > 0.999f && Diffuse0.y > 0.999f && Diffuse0.z > 0.999f)
				fragment_color = fogColor;
			else if(len > maxdist)
				fragment_color = fogColor;
			else
			{
				Position0 = texture(Position, vec2(UV.x, UV.y));
				Normal0 = texture(Normal, vec2(UV.x, UV.y));
				Depth0 = texture(Depth, vec2(UV.x, UV.y));

				for(int n = 0; n < NumSpotLights; n++)
				{
					if(n < NumSpotLightsShadow)
					{
					
						fragment_color += CalcSpotLight(lights[n], Normal0.xyz) * CalcShadowFactor();
					}
					else
						fragment_color += CalcSpotLight(lights[n], Normal0.xyz);
				}
				
				fragment_color =  fogColor * (len/maxdist) + (1-len/maxdist) * fragment_color * Diffuse0; // 
			}
		}
		else
		{
			fragment_color = vec4(0,1,1,1);		
		}
	}
)";

	GLint success = 0;

	//create vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, nullptr);
	glCompileShader(vs);
	CompileErrorPrint(&vs);

	//create fragment shader
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, nullptr);
	glCompileShader(fs);
	CompileErrorPrint(&fs);

	//link shader program (connect vs and ps)
	*gShaderProgram = glCreateProgram();
	glAttachShader(*gShaderProgram, vs);
	glAttachShader(*gShaderProgram, fs);
	glLinkProgram(*gShaderProgram);
	LinkErrorPrint(gShaderProgram);

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
	return true;
}

void ShaderBlit::CompileErrorPrint(GLuint* shader)
{
	GLint success = 0;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(*shader, maxLength, &maxLength, &errorLog[0]);

		std::fstream myfile;
		myfile.open("errorCheck.txt", std::fstream::out);
		for (int i = 0; i < maxLength; i++)
		{
			myfile << errorLog[i];
		}
		myfile.close();

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(*shader); // Don't leak the shader.
		throw;
	}
}

void ShaderBlit::LinkErrorPrint(GLuint* shaderProgram)
{
	GLint success = 0;
	glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(*shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(*shaderProgram, maxLength, &maxLength, &errorLog[0]);

		std::fstream myfile;
		myfile.open("errorCheck.txt", std::fstream::out);
		for (int i = 0; i < maxLength; i++)
		{
			myfile << errorLog[i];
		}
		myfile.close();

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteProgram(*shaderProgram); // Don't leak the shader.

		if (success == GL_FALSE)
			throw;
	}
}