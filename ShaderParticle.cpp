#include "ShaderParticle.h"
#include <fstream>
#include <iostream>
#include <vector>

ShaderParticle::ShaderParticle(GLuint* gShaderP)
{
	gShaderProgram = gShaderP;
	compile();
	ViewMatrix = glGetUniformLocation(*gShaderProgram, "ViewMatrix");
	ProjectionMatrix = glGetUniformLocation(*gShaderProgram, "ProjectionMatrix");
}

bool ShaderParticle::compile()
{
	const char* vertex_shader = R"(
		#version 430

		uniform mat4 ViewMatrix;
		
		struct particles
		{
			vec3 pos;
			float xPos;
			vec3 velocity;
			float yPos;
		};

		layout (std430, binding = 2) buffer particlesArray
		{
			particles data[];
		};

		void main () 
		{
			gl_Position = ViewMatrix * vec4(data[gl_VertexID].pos, 1);
		}
	)";

	const char* geometry_shader = R"( 
		#version 430
		layout (points) in;
		layout(triangle_strip, max_vertices = 4) out;

		uniform mat4 ProjectionMatrix;

		void main ()
		{
			vec3 nPos = normalize(gl_in[0].gl_Position.xyz);
			vec3 up = vec3(0.0f, 1.0f, 0.0f);
			vec3 bBoardVec = cross(-nPos, up);
	
			//Triangle
			//Upper left vertex(corner)
			gl_Position = vec4( gl_in[0].gl_Position.xyz - bBoardVec*0.15f + vec3(0.0f, 0.15f, 0.0f), 1.0f );
			gl_Position = ProjectionMatrix * gl_Position;
			EmitVertex();

			//Lower left vertex(corner)
			gl_Position = vec4( gl_in[0].gl_Position.xyz - bBoardVec*0.15f - vec3(0.0f, 0.15f, 0.0f), 1.0f );
			gl_Position = ProjectionMatrix * gl_Position;
			EmitVertex();

			//Upper right vertex(corner)
			gl_Position = vec4( gl_in[0].gl_Position.xyz + bBoardVec*0.15f + vec3(0.0f, 0.15f, 0.0f), 1.0f );
			gl_Position = ProjectionMatrix * gl_Position;	
			EmitVertex();

			//Lower right vertex(corner)
			gl_Position = vec4( gl_in[0].gl_Position.xyz + bBoardVec*0.15f - vec3(0.0f, 0.15f, 0.0f), 1.0f );
			gl_Position = ProjectionMatrix * gl_Position;
			EmitVertex();

			EndPrimitive();
		}
	)";

	const char* fragment_shader = R"(
		#version 430

		layout (location = 0) out vec3 WorldPosOut;   
		layout (location = 1) out vec3 DiffuseOut;     
		layout (location = 2) out vec3 NormalOut;     
		layout (location = 3) out vec3 TexCoordOut;    

		void main () {
			WorldPosOut = vec3(1,0,0);
			DiffuseOut = vec3(0,0,1);
			NormalOut = vec3(1,0,0);
			TexCoordOut = vec3 (1,0,0);	
		}
	)";

	GLint success;

	//Vertex Shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, nullptr);
	glCompileShader(vs);
	CompileErrorPrint(&vs);

	//Geometry Shader
	GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(gs, 1, &geometry_shader, nullptr);
	glCompileShader(gs);
	CompileErrorPrint(&gs);

	//create fragment shader
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, nullptr);
	glCompileShader(fs);
	CompileErrorPrint(&fs);

	*gShaderProgram = glCreateProgram();
	glAttachShader(*gShaderProgram, vs);
	glAttachShader(*gShaderProgram, gs);
	glAttachShader(*gShaderProgram, fs);
	glLinkProgram(*gShaderProgram);

	LinkErrorPrint(gShaderProgram);

	return true;
}

void ShaderParticle::CompileErrorPrint(GLuint* shader)
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

void ShaderParticle::LinkErrorPrint(GLuint* shaderProgram)
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
