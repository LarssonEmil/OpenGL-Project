#include "HeightMapdata.h"
#include "stb_image.h"
#define BUFFER_OFFSET(i) ((char *)nullptr + (i))
#define GLM_FORCE_RADIANS

HeightMapdata::HeightMapdata()
{
	texWidth = 0;
	texHeight = 0;
	comp = 0;
	pixels = 0;
	gridWidth = 257;
	gridHeight = 257;
	gridSize = gridWidth*gridHeight;
	gIndexBuffer = 0;
	gIndexAttribute = 0;
	IBOCounter = 0;
	startingPosX = 0;
	startingPosZ = 0;
}

HeightMapdata::~HeightMapdata()
{
	delete heightMap;
}

void HeightMapdata::Init()
{
	loadImage();
	createRealHeightMap();
	//sampleHeightMaptoGetNormal();
	//createIBO();
	createIBOsubs();
	HeightMapBuffers();
	loadMapTextures();
	createShaderBuffer();
}

void HeightMapdata::Bind(GLuint* shaderProgram, ShaderHMap* shader)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, getTexture());
	glProgramUniform1i(*shaderProgram, shader->heightMapSampler, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grass_texture);
	glProgramUniform1i(*shaderProgram, shader->grassSampler, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, road_texture);
	glProgramUniform1i(*shaderProgram, shader->roadSampler, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, blend_texture);
	glProgramUniform1i(*shaderProgram, shader->blendMapSampler, 3);
}

void HeightMapdata::createShaderBuffer() 
{
	y = new float();
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), y, GL_DYNAMIC_READ);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo);
}

void HeightMapdata::loadMapTextures()
{
	int comp, comp2, comp3;
	const char* filename = new char();
	const char* filename2 = new char();
	const char* filename3 = new char();
	filename = "grassTexture.png";//"grass.png";
	filename2 = "dirtRoad.png";//"road.png";
	filename3 = "blendMap.png";
	GLubyte* imageGrass = stbi_load(filename, &grassWidth, &grassHeight, &comp, 4);
	GLubyte* imageRoad = stbi_load(filename2, &roadWidth, &roadHeight, &comp2, 4);
	GLubyte* imageBlendMap = stbi_load(filename3, &blendMapWidth, &blendMapHeight, &comp3, 4);

	if (imageGrass == nullptr)
		throw(std::string("Failed to load texture"));
	if (imageRoad == nullptr)
		throw(std::string("Failed to load texture"));
	if (imageBlendMap == nullptr)
		throw(std::string("Failed to load texture"));

	createMapTexture(imageGrass, imageRoad, imageBlendMap);

	stbi_image_free(imageGrass);
	stbi_image_free(imageRoad);
	stbi_image_free(imageBlendMap);
}

void HeightMapdata::createMapTexture(GLubyte* imageGrass, GLubyte* imageRoad, GLubyte* imageBlendMap)
{
	//create grass texture
	glGenTextures(1, &grass_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grass_texture);

	glTexStorage2D(GL_TEXTURE_2D, 8, GL_RGBA8, grassWidth, grassHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, grassWidth, grassHeight, GL_BGRA, GL_UNSIGNED_BYTE, (const void*)imageGrass);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//create road texture
	glGenTextures(1, &road_texture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, road_texture);

	glTexStorage2D(GL_TEXTURE_2D, 8, GL_RGBA8, roadWidth, roadHeight);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, roadWidth, roadHeight, GL_BGRA, GL_UNSIGNED_BYTE, (const void*)imageRoad);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//create blend map texture
	glGenTextures(1, &blend_texture);
	glActiveTexture(GL_TEXTURE3);

	glBindTexture(GL_TEXTURE_2D, blend_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, blendMapWidth, blendMapHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageBlendMap);

	mat1Scale = 16;
	mat2Scale = 16;
}

void HeightMapdata::HeightMapBuffers()
{
	glGenBuffers(1, &gHeightMapBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, gHeightMapBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*(gridSize), &heightMap[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &gHeightMapAttribute);
	glBindVertexArray(gHeightMapAttribute);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(0));

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(sizeof(float) * 3));

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}

void HeightMapdata::loadImage()
{	
	const char* filename = new char();
	filename = "heightmap1.png";
	GLubyte* image = stbi_load(filename, &texWidth, &texHeight, &comp, 1); //change the last 0 to 1

	if (image == nullptr)
		throw(std::string("Failed to load texture"));

	createTexture(image);
	
	stbi_image_free(image);

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}

void HeightMapdata::createTexture( GLubyte* image )
{
	glGenTextures(1, &height_texture);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, height_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texWidth, texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, image); //GL_R

#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}

void HeightMapdata::createRealHeightMap()
{
	heightMap = new Vertex[gridSize];
	for (int column = 0; column < gridHeight; column++)
	{
		for (int row = 0; row < gridWidth; row++)
		{
			heightMap[column*gridWidth + row] = Vertex((float)row, 1.0f, (float)column, (float)row / (float)gridWidth, (float)column / (float)gridHeight);
		}
	}
#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif

}

void HeightMapdata::createIBO()
{
	std::vector<unsigned int> IBO;
	for (int i = 0; i < gridHeight - 1; i++)
	{
		if ((i % 2) == 0)
		{
			for (int j = 0; j < gridWidth; j++)
			{
				IBO.push_back(i*gridWidth + j);
				IBO.push_back((i + 1)*gridWidth + j);
			}
		}
		else
		{
			for (int j = gridWidth - 1; j >= 0; j--)
			{
				IBO.push_back(i*gridWidth + j);
				IBO.push_back((i + 1)*gridWidth + j);
			}
		}
		IBO.push_back(IBO.back());
	}
	IBOCounter = IBO.size();

	glGenBuffers(1, &gIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, IBOCounter * sizeof(unsigned int), &IBO[0], GL_STATIC_DRAW);


#ifdef _DEBUG
	{GLenum err = glGetError(); if (err)
		int x = 0; }
#endif
}

void HeightMapdata::createIBOsubs()
{
	chunksize = 8;
	int buffers = (gridWidth / chunksize) * (gridWidth / chunksize);
	subIndexBuffers = new GLuint[buffers];

	int count = 0;
	for (int y = 0; y < (gridWidth / chunksize) ; y++)
	{
		for (int x = 0; x < (gridWidth / chunksize) ; x++)
		{
			IndexingSubArea(&subIndexBuffers[count++], x, y, gridWidth, chunksize);
		}
	}
}

void HeightMapdata::IndexingSubArea(GLuint* indexBuff, int x, int y, int widthMAP, int widthChunk)
{
	int sx = (x * widthChunk + y * widthMAP * widthChunk);
	int sx2 = (x * widthChunk + (y * widthMAP * widthChunk + widthMAP));

	//sizes:
	// 4x4 = 40
	// 8x8 = 288

	std::vector<unsigned int> IBO;
	//IBO.reserve(136);
	count = 0;
	for (int n = 0; n < (widthChunk / 2); n++)
	{
		for (int j = 0; j < widthChunk + 1; j++)
		{
			IBO.push_back(sx + j);
			IBO.push_back(sx2 + j);
			count += 2;
		}
		IBO.push_back(sx2 + 8);
		sx += widthMAP * 2;
		count ++;

		for (int j = 0; j < widthChunk + 1; j++)
		{
			IBO.push_back(sx2 + ( widthChunk) - j);
			IBO.push_back(sx + (widthChunk) - j);
			count += 2;
		}
		IBO.push_back(sx);
		sx2 += widthMAP * 2;
		count++;
	}

	glGenBuffers(1, indexBuff);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexBuff);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), &IBO[0], GL_STATIC_DRAW);
}

bool HeightMapdata::terrainCollison(glm::vec3 camerapos)        
{
	float mapEndX = getStartingX() + getGridWidth();
	float mapEndZ = getStartingZ() + getGridHeight();

	if (camerapos.x >= getStartingX() && camerapos.x <= mapEndX && camerapos.z >= getStartingZ() && camerapos.z <= mapEndZ)
	{
		float cameraU = camerapos.x / getGridWidth();
		float cameraV = camerapos.z / getGridHeight();
		cameraUV = glm::vec2(cameraU, cameraV);
		return true;
	}
	return false;
}
//
//void HeightMapdata::sampleHeightMaptoGetNormal()
//{
//	//tmpBufferArray = new glm::vec3[gridSize];
//	glm::vec3 tmpBufferArray[66049];
//	glm::vec3 closestPArray[5]; // index 0 is the point we are in currently.
//	glm::vec3 averageNormal;
//	int counter = 0;
//
//	for (int i = 0; i < gridWidth; i++)
//	{
//		for (int j = 0; j < gridHeight; j++)
//		{
//			averageNormal = glm::vec3(0, 0, 0);
//
//			if (i != 0 && i != 256 && j != 0 && j != 256) //regular way
//			{
//				closestPArray[0] = heightMap[i * gridWidth + j].getXYZ();
//				closestPArray[1] = heightMap[(i * gridWidth + j) - 257].getXYZ();
//				closestPArray[2] = heightMap[(i * gridWidth + j) - 1].getXYZ();
//				closestPArray[3] = heightMap[(i * gridWidth + j) + 1].getXYZ();
//				closestPArray[4] = heightMap[(i * gridWidth + j) + 257].getXYZ();
//			}
//			if (i == 0)
//			{
//				closestPArray[0] = heightMap[i * gridWidth + j].getXYZ();
//				closestPArray[1] = glm::vec3(0, 1, 0);
//				closestPArray[2] = heightMap[(i * gridWidth + j) - 1].getXYZ();
//				closestPArray[3] = heightMap[(i * gridWidth + j) + 1].getXYZ();
//				closestPArray[4] = heightMap[(i * gridWidth + j) + 257].getXYZ();
//			}
//			else if (i == 256)
//			{
//				closestPArray[0] = heightMap[i * gridWidth + j].getXYZ();
//				closestPArray[1] = heightMap[(i * gridWidth + j) - 257].getXYZ();
//				closestPArray[2] = heightMap[(i * gridWidth + j) - 1].getXYZ();
//				closestPArray[3] = heightMap[(i * gridWidth + j) + 1].getXYZ();
//				closestPArray[4] = glm::vec3(0, 1, 0);
//			}
//
//			if (j == 0)
//			{
//				closestPArray[0] = heightMap[i * gridWidth + j].getXYZ();
//				if (i == 0)
//					closestPArray[1] = glm::vec3(0, 1, 0);
//				else
//					closestPArray[1] = heightMap[(i * gridWidth + j) - 257].getXYZ();
//				closestPArray[2] = glm::vec3(0, 1, 0);
//				closestPArray[3] = heightMap[(i * gridWidth + j) + 1].getXYZ();
//				if (i == 256)
//					closestPArray[4] = glm::vec3(0, 1, 0);
//				else
//					closestPArray[4] = heightMap[(i * gridWidth + j) + 257].getXYZ();
//			}
//			else if (j == 256)
//			{
//				closestPArray[0] = heightMap[i * gridWidth + j].getXYZ();
//				if (i == 0)
//					closestPArray[1] = glm::vec3(0, 1, 0);
//				else
//					closestPArray[1] = heightMap[(i * gridWidth + j) - 257].getXYZ();
//				closestPArray[2] = heightMap[(i * gridWidth + j) - 1].getXYZ();	
//				closestPArray[3] = glm::vec3(0, 1, 0);
//				if (i == 256)
//					closestPArray[4] = glm::vec3(0, 1, 0);
//				else
//					closestPArray[4] = heightMap[(i * gridWidth + j) + 257].getXYZ();
//			}
//
//			for (int n = 1; n < 5; n++)
//			{
//				if (n%2)
//					averageNormal += normalize(cross(closestPArray[0], closestPArray[n]));
//				else
//					averageNormal += normalize(cross(closestPArray[n], closestPArray[0]));
//			}
//			/*averageNormal += normalize(cross(closestPArray[1], closestPArray[2]));
//			averageNormal += normalize(cross(closestPArray[2], closestPArray[3]));
//			averageNormal += normalize(cross(closestPArray[3], closestPArray[4]));
//			averageNormal += normalize(cross(closestPArray[4], closestPArray[1]));*/
//
//			averageNormal = averageNormal / 4.0f;
//
//			tmpBufferArray[counter] = normalize(averageNormal); //Array with vertex normals.
//			counter++;
//		}
//	}
//
//	for (int column = 0; column < gridWidth; column++)
//	{
//		for (int row = 0; row < gridHeight; row++)
//		{
//			//heightMap[column*gridWidth + row].setNormals(glm::vec3(1,0,0));
//			heightMap[column*gridWidth + row].setNormals(tmpBufferArray[column*gridWidth + row]);
//		}
//	}
//}
//

int HeightMapdata::getIBOCount()
{
	return IBOCounter;
}

int HeightMapdata::getGridSize()
{
	return gridSize;
}

int HeightMapdata::getGridWidth()
{
	return gridWidth;
}

int HeightMapdata::getGridHeight()
{
	return gridHeight;
}

Vertex* HeightMapdata::getRealHeightMap()
{
	return heightMap;
}

GLuint HeightMapdata::getTexture()
{
	return height_texture;
}

float HeightMapdata::getStartingX()
{
	return startingPosX;
}

float HeightMapdata::getStartingZ()
{
	return startingPosZ;
}
