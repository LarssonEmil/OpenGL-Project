#include "ShaderHMap.h"
#include <fstream>
#include <iostream>
#include <vector>

ShaderHMap::ShaderHMap(GLuint* gShaderP)
{
	gShaderProgram = gShaderP;
	compile();
	ViewMatrix = glGetUniformLocation(*gShaderProgram, "ViewMatrix");
	ProjectionMatrix = glGetUniformLocation(*gShaderProgram, "ProjectionMatrix");
	heightMapSampler = glGetUniformLocation(*gShaderProgram, "heightMapSampler");
	grassSampler = glGetUniformLocation(*gShaderProgram, "grassSampler");
	roadSampler = glGetUniformLocation(*gShaderProgram, "roadSampler");
	blendMapSampler = glGetUniformLocation(*gShaderProgram, "blendMapSampler");
	cameraUV = glGetUniformLocation(*gShaderProgram, "cameraUV");
	mat1Scale = glGetUniformLocation(*gShaderProgram, "mat1Scale");
	mat2Scale = glGetUniformLocation(*gShaderProgram, "mat2Scale");
}

bool ShaderHMap::compile()
{
	const char* vertex_shader = R"(
		#version 430
		layout(location = 0) in vec3 heightMapPos;
		layout(location = 1) in vec2 heightMapUV;
		layout(location = 2) in vec3 smoothNormals;

		uniform mat4 ViewMatrix;
		
		uniform sampler2D heightMapSampler;
		uniform vec2 cameraUV;

		layout (std430, binding = 4) buffer y
		{
			float heightS;
		};

		out vec2 texCoords;
		out vec3 camera;
		out vec3 posi;
		out vec3 SNormals;
		
		void main () {
			posi = vec3(heightMapPos.x, texture(heightMapSampler, heightMapUV).x*30 ,heightMapPos.z);
			gl_Position = ViewMatrix * vec4( posi, 1 );
			texCoords = heightMapUV;
			SNormals = smoothNormals;

			vec4 heightTmp = texture( heightMapSampler, cameraUV );
			heightS = heightTmp.x*50;
		}
	)";

	const char* geometry_shader = R"( 
		#version 430
		layout (triangles) in;
		layout (triangle_strip) out;
		layout(max_vertices = 3) out;

		in vec2 texCoords[];
		in vec3 posi[];
		in vec3 SNormals[];

		uniform mat4 ProjectionMatrix;

		out vec2 texCoordsGeo;
		out vec3 possi;
		out vec3 normalWorld;

		void main ()
		{
			vec3 v1 = gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz;
			vec3 v2 = gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz;
			vec3 normal = cross(v1, v2);

			//back face culling
			if( dot(gl_in[0].gl_Position.xyz, normal) < 0.0f )
			{
				v1 = posi[1] - posi[0];
				v2 = posi[2] - posi[0];

				for ( int i = 0; i < gl_in.length(); i++ )
				{
					gl_Position = ProjectionMatrix * gl_in[i].gl_Position;
					texCoordsGeo = texCoords[i];
					possi = posi[i];
					normalWorld = normalize(cross(v1, v2)); //SNormals[i];//
					EmitVertex();
				}
				EndPrimitive();
			}		
		}
	)";

	const char* fragment_shader = R"(
		#version 430
		in vec2 texCoordsGeo;
		in vec3 possi;
		in vec3 normalWorld;	

		uniform sampler2D heightMapSampler;

		uniform sampler2D grassSampler;
		uniform sampler2D roadSampler;	
		uniform sampler2D blendMapSampler;	
		uniform float mat1Scale;
		uniform float mat2Scale;

		layout (location = 0) out vec3 WorldPosOut;   
		layout (location = 1) out vec3 DiffuseOut;     
		layout (location = 2) out vec3 NormalOut;     
		layout (location = 3) out vec3 TexCoordOut;    

		void main () {
			vec3 blendMap = texture(blendMapSampler ,vec2(texCoordsGeo)).xyz;
			vec3 mat1 = texture(grassSampler, texCoordsGeo*mat1Scale).xy<;
			vec3 mat2 = texture(roadSampler, texCoordsGeo*mat2Scale);

			WorldPosOut = possi;
			DiffuseOut = (blendMap.g*mat1 + blendMap.r*mat2).xyz; //DiffuseOut = -0.15f + (possi / 256.0f);
			NormalOut = normalWorld;
			TexCoordOut = vec3 (1,0,0);	
		}
	)";

	GLint success = 0;

	//create vertex shader
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, nullptr);
	glCompileShader(vs);
	CompileErrorPrint(&vs);

	GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
	glShaderSource(gs, 1, &geometry_shader, nullptr);
	glCompileShader(gs);
	CompileErrorPrint(&gs);

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

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
		printf("Error");
	return true;
}

void ShaderHMap::LinkErrorPrint(GLuint* shaderProgram)
{
	GLint success;
	glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(*shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetProgramInfoLog(*shaderProgram, maxLength, &maxLength, &errorLog[0]);

		std::fstream myfile;
		myfile.open("errorLinkCheck.txt", std::fstream::out);
		for (int i = 0; i < maxLength; i++)
		{
			myfile << errorLog[i];
		}
		myfile.close();

		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteProgram(*shaderProgram); // Don't leak the shader.
		throw;
	}
}

void ShaderHMap::CompileErrorPrint(GLuint* shader)
{
	GLint success;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv((*shader), GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog((*shader), maxLength, &maxLength, &errorLog[0]);

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