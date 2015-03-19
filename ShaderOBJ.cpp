#include "ShaderOBJ.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>

ShaderOBJ::ShaderOBJ(GLuint* gShaderP)
{
	gShaderProgram = gShaderP;
	compile();

	model = glGetUniformLocation(*gShaderProgram, "World");
	view = glGetUniformLocation(*gShaderProgram, "View");
	proj = glGetUniformLocation(*gShaderProgram, "Projection");
	textureSample = glGetUniformLocation(*gShaderProgram, "textureSample");
	
#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}

bool ShaderOBJ::compile()
{
	const char* vertex_shader = R"(
	#version 410
	layout (location = 0) in vec3 Position;                                             
	layout (location = 1) in vec2 UV;                                             

	uniform mat4 World;
	uniform mat4 View;
	uniform mat4 Projection;
                                        
	layout (location = 0) out vec3 Position0;                                                                 
	layout (location = 1) out vec2 UV0;                                                                                                                                 

	void main()
	{       
		Position0		= (vec4(Position, 1.0) * World).xyz; 
		gl_Position		= Projection * View * vec4(Position0, 1); 
		UV0				= UV;    
	}
)";

	const char* geometry_shader = R"(
	#version 410
	layout(triangles) in;
	layout(triangle_strip, max_vertices = 3) out;

	layout (location = 0) in vec3 Position[3];
	layout (location = 1) in vec2 UVCord[3];

	layout (location = 0) out vec3 Positions;
	layout (location = 1) out vec2 UV;
	layout (location = 2) out vec3 Normal;

	void main() 
	{
		vec3 v1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
		vec3 v2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
		vec3 normal = cross(v1, v2);

		//back face culling
		if( dot(gl_in[0].gl_Position.xyz, normal) > 0.0f )
		{
			vec3 line1 = vec3(Position[1] - Position[0]);
			vec3 line2 = vec3(Position[2] - Position[0]);
			Normal = normalize(cross(line1, line2));

			for(int n = 0; n < 3; n++)
			{
				Positions = Position[n];
				gl_Position = gl_in[n].gl_Position;
				UV = UVCord[n];
				EmitVertex();
			}
			EndPrimitive();
		}
	}
)";


	const char* fragment_shader = R"(
	#version 410
	layout (location = 0) in vec3 Position;                                                                  
	layout (location = 1) in vec2 UV;                                                                  
	layout (location = 2) in vec3 Normal;                                                                    

	uniform sampler2D textureSample;

	layout (location = 0) out vec3 WorldPosOut;   
	layout (location = 1) out vec3 DiffuseOut;     
	layout (location = 2) out vec3 NormalOut;     
	layout (location = 3) out vec3 TexCoordOut;    
																				
	void main()									
	{											
		WorldPosOut     = Position;					
		DiffuseOut      = texture(textureSample, UV).xyz;	
		NormalOut       = Normal;					
		TexCoordOut     = vec3(mod(	UV.x, 1.0), mod(UV.y, 1.0), 0.0);				
	}
)";
		GLint success = 0;

		//create vertex shader
		GLuint vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vertex_shader, nullptr);
		glCompileShader(vs);
		CompileErrorPrint(&vs);

		//create fragment shader
		GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(gs, 1, &geometry_shader, nullptr);
		glCompileShader(gs);
		CompileErrorPrint(&gs);

		//create fragment shader
		GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fs, 1, &fragment_shader, nullptr);
		glCompileShader(fs);
		CompileErrorPrint(&fs);

		//link shader program (connect vs and ps)
		*gShaderProgram = glCreateProgram();
		glAttachShader(*gShaderProgram, vs);
		glAttachShader(*gShaderProgram, gs);
		glAttachShader(*gShaderProgram, fs);
		glLinkProgram(*gShaderProgram);
		LinkErrorPrint(gShaderProgram);

#ifdef _DEBUG
		{GLenum err = glGetError(); if (err)
			int x = 0; }
#endif
		return true;
}

void ShaderOBJ::CompileErrorPrint(GLuint* shader)
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

void ShaderOBJ::LinkErrorPrint(GLuint* shaderProgram)
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