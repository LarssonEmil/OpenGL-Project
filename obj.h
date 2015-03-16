#ifndef OBJ_H
#define OBJ_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#define BUFFER_OFFSET(i) ((char *)nullptr + (i))

class Obj
{
private:
	const float toRad = 0.0174532925;

	GLuint vertexDataId;
	GLuint gVertexAttribute;
	GLuint textureId;
	GLuint* gShaderProgram;
	bool loadVert(const std::string);
	void loadVert2();
	bool loadBMP(const std::string);
	
public:
	int id;
	std::string name;
	GLuint IndexBufferId;
	
	struct TriangleVertex
	{
		float x, y, z;
		float u, v;
	};
	Obj(const char* name, int id, float scale);
	glm::vec4 pos;
	float rotx, roty, rotz;
	void translate(float x, float y, float z);
	void rotate(float x, float y, float z);
	void scaleUniform(float x);
	int Bind();
	void Unbind();
	int vertCount;
	int faceCount;
	float scale;
	glm::mat4 worldMatrix = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f,
										0.0f, 1.0f, 0.0f, 0.0f,
										0.0f, 0.0f, 1.0f, 0.0f,
										0.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 normalMatrix = glm::inverse(glm::transpose(worldMatrix));
};

#endif