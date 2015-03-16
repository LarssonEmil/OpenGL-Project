#include "obj.h"
#include <vector>

using namespace std;
Obj::Obj(const char* name, int id)
{
	this->id = id;
	std::string temp = name;
	temp.append(".v");
	std::string temp2 = name;
	temp2.append(".bmp");
	
	rotx = roty = rotz = 0.0f;

	//load triangle data
	if (temp == "boat.v")
		loadVert2();
	else
		if (!loadVert(temp))
			throw;
	//load texture data
	if (!loadBMP(temp2))
		throw;
}

bool Obj::loadBMP(const std::string imagepath)
{
	unsigned char header[54]; // Each BMP file begins by a 54-bytes header
	unsigned int dataPos;     // Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize;   // = width*height*3
	// Actual RGB data
	unsigned char * data;

	FILE* file = fopen(imagepath.c_str(), "rb");
	if (!file)
		return false;

	if (fread(header, 1, 54, file) != 54) // If not 54 bytes read : problem
		return false;

	if (header[0] != 'B' || header[1] != 'M')
		return false;

	// Read ints from the byte array
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)
		imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)
		dataPos = 54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	//Everything is in memory now, the file can be closed
	fclose(file);

	// Create one OpenGL texture
	glGenTextures(1, &textureId);

	glActiveTexture(GL_TEXTURE0 + 6 + id);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureId);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	delete[] data;

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif

	return true;
}

bool Obj::loadVert(const std::string vertdatapath)
{
	std::string line;
	std::ifstream myfile(vertdatapath);
	if (myfile.is_open())
	{
		getline(myfile, line);
		int vert_count = atoi(line.c_str());
		TriangleVertex* vert = new TriangleVertex[vert_count];

		for (int n = 0; n < vert_count; n++)
		{
			//read vert data, seperate with space
			getline(myfile, line);
			std::istringstream iss(line);
			std::string sub;
			//vert
			iss >> sub;
			vert[n].x = std::stof(sub);
			iss >> sub;
			vert[n].y = std::stof(sub);
			iss >> sub;
			vert[n].z = std::stof(sub);
			getline(myfile, line);
			std::istringstream iss2(line);
			iss2.str(line);
			//uv
			iss2 >> sub;
			vert[n].u = std::stof(sub);
			iss2 >> sub;
			vert[n].v = std::stof(sub);
		}
		//load index buffer (faces)
		getline(myfile, line);
		int face_count = atoi(line.c_str());
		GLushort* Indices = new GLushort[face_count * 3];

		for (int n = 0; n < face_count; n++)
		{
			//read index data, seperate with space
			getline(myfile, line);
			std::istringstream iss(line);
			std::string sub;
			//vert
			iss >> sub;
			Indices[n * 3] = std::stoi(sub)-1;
			iss >> sub;
			Indices[n * 3 + 1] = std::stoi(sub) - 1;
			iss >> sub;
			Indices[n * 3 + 2] = std::stoi(sub) - 1;
		}
		
		glGenBuffers(1, &vertexDataId);
		glBindBuffer(GL_ARRAY_BUFFER, vertexDataId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vert[0])* vert_count, vert, GL_STATIC_DRAW);
		vertCount = vert_count;
		delete[] vert;

		glGenBuffers(1, &IndexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * face_count * 3, Indices, GL_STATIC_DRAW);
		faceCount = face_count;
		delete [] Indices;
		glEnableVertexAttribArray(IndexBufferId);

		//define vertex data layout
		glGenVertexArrays(1, &gVertexAttribute);
		glBindVertexArray(gVertexAttribute);
		glEnableVertexAttribArray(0); //the vertex attribute object will remember its enabled attributes
		glEnableVertexAttribArray(1);
		//pos
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Obj::TriangleVertex), BUFFER_OFFSET(0));
		//uv
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Obj::TriangleVertex), BUFFER_OFFSET(sizeof(float) * 3));
#ifdef _DEBUG
		{GLenum err = glGetError(); if (err)
			int x = 0; }
#endif
	}
	else
		return false;
	myfile.close();
	return true;
	
}

void Obj::loadVert2()
{
	std::string line;
	std::ifstream myfile("boat2.v");
	if (myfile.is_open())
	{
		std::vector<TriangleVertex> vert;
		std::vector<TriangleVertex> uv;
		std::vector<GLushort> Indices;

		std::string sub;
		int count = 0;
		int state = 0;
		while (true) {
			if (!(getline(myfile, line))) break;
			if (line.size() < 5)
				continue;
			if (line[0] == 'v' && line[1] == ' ') // vertex pos
			{
				vert.push_back(TriangleVertex());

				std::istringstream iss(line);
				std::string sub;
				iss >> sub; // discard 'v'
				iss >> sub;
				vert[count].x = std::stof(sub);
				iss >> sub;
				vert[count].y = std::stof(sub);
				iss >> sub;
				vert[count].z = std::stof(sub);
				vert[count].u = 0.0f;
				vert[count].v = 0.0f;
				count++;
			}
			else if (line[0] == 'v' && line[1] == 't') //UV cord
			{
				if (state != 1)
				{
					state = 1;
					count = 0;
				}
				uv.push_back(TriangleVertex());
				std::istringstream iss(line);
				std::string sub;
				iss >> sub; // discard 'vt'
				iss >> sub;
				uv[count].u = std::stof(sub);
				iss >> sub;
				uv[count].v = std::stof(sub);
				count++;
			}
			else if (line[0] == 'f') // face
			{
				if (state != 2)
				{
					state = 2;
					count = 0;
				}
				std::istringstream iss(line);
				std::string sub;
				Indices.push_back(0);
				Indices.push_back(0);
				Indices.push_back(0);
				//vert
				iss >> sub; // discard 'f'
				iss >> sub; //cord 1
				Indices[count * 3] = std::stoi(sub) - 1;
				int index = std::stoi(sub) - 1;
				iss >> sub; // uv 1
				vert[index].u = uv[std::stoi(sub) - 1].u;
				vert[index].v = uv[std::stoi(sub) - 1].v;
				iss >> sub; // normal 1

				iss >> sub; // cord 2
				Indices[count * 3 + 1] = std::stoi(sub) - 1;
				index = std::stoi(sub) - 1;
				iss >> sub; // uv 1
				vert[index].u = uv[std::stoi(sub) - 1].u;
				vert[index].v = uv[std::stoi(sub) - 1].v;
				iss >> sub; // normal 2

				iss >> sub; // cord 3
				Indices[count * 3 + 2] = std::stoi(sub) - 1;
				index = std::stoi(sub) - 1;
				iss >> sub; // uv 1
				vert[index].u = uv[std::stoi(sub) - 1].u;
				vert[index].v = uv[std::stoi(sub) - 1].v;
				
				iss >> sub; // normal 3
				count++;
			}
		}

		glGenBuffers(1, &vertexDataId);
		glBindBuffer(GL_ARRAY_BUFFER, vertexDataId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vert[0])* vert.size(), &vert[0], GL_STATIC_DRAW);

		glGenBuffers(1, &IndexBufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices[0]) * count * 3, &Indices[0], GL_STATIC_DRAW);
		faceCount = count;
		glEnableVertexAttribArray(IndexBufferId);
		
		//define vertex data layout
		glGenVertexArrays(1, &gVertexAttribute);
		glBindVertexArray(gVertexAttribute);
		glEnableVertexAttribArray(0); //the vertex attribute object will remember its enabled attributes
		glEnableVertexAttribArray(1);
		//pos
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Obj::TriangleVertex), BUFFER_OFFSET(0));
		//uv
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Obj::TriangleVertex), BUFFER_OFFSET(sizeof(float) * 3));
#ifdef _DEBUG
		{GLenum err = glGetError(); if (err)
			int x = 0; }
#endif
	}
	else
		myfile.close();

}

int Obj::Bind()
{
	glActiveTexture(GL_TEXTURE0 + 6 + id);
	glBindTexture(GL_TEXTURE_2D, textureId);
	glBindVertexArray(gVertexAttribute);
	glBindBuffer(GL_ARRAY_BUFFER, vertexDataId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId);
#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
	return 1;
}

void Obj::rotate(float x, float y, float z, bool local)
{
	if (local == false) //rotate order x, y, z	
	{
		float rotx = toRad * x;
		float roty = toRad * y;
		float rotz = toRad * z;

		//rot Y
		worldMatrix *= glm::mat4(cos(roty), 0.0f, -sin(roty), 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 
			sin(roty), 0.0f, cos(roty), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
	}
}