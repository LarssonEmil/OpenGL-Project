#ifndef HEIGHTMAPDATA_H
#define HEIGHTMAPDATA_H

#include <string>
#include "Vertex.h"
#include "ShaderHMap.h"
#include <vector>

#include <gl/glew.h>
#include <gl/GL.h>

class HeightMapdata
{
	public:
		HeightMapdata();
		~HeightMapdata();

		void Init();
		void HeightMapBuffers();
		void loadMapTextures();
		void createMapTexture(GLubyte* imageGrass, GLubyte* imageRoad, GLubyte* imageBlendMap);
		void createShaderBuffer();
		void Bind(GLuint* shaderProgram, ShaderHMap* shader);
		bool HeightMapdata::terrainCollison(glm::vec3 cameraPos);
		void IndexingSubArea(GLuint* indexBuff, int x, int y, int sidesHMap, int sidesChunk);
		void createIBOsubs();
		int count;
		int chunksize;

		void loadImage();  //Loads the image we want to use.
		void createTexture( GLubyte* image );  //Makes the image/preHeightMap to a texture.
		void createRealHeightMap(); //Creates the actual heightmap
		void createIBO();  //Creates a buffer that holds in which order the vertices should be rendered.
		void sampleHeightMaptoGetNormal(); //Calculate smooth normals for the heightmap

		//Get functions
		int getIBOCount();
		int getGridSize();
		int getGridWidth();
		int getGridHeight();
		float getStartingX();
		float getStartingZ();
		GLuint getTexture();
		

		Vertex* getRealHeightMap();

		//Index buffer, saying in which order the indexes in ssbo should be writen.
		GLuint gIndexBuffer;
		GLuint *subIndexBuffers;

		//heightmap uniforms
		glm::vec2 cameraUV;
		float mat1Scale;
		float mat2Scale;

		//Buffers 
		GLuint gHeightMapBuffer;
		GLuint gHeightMapAttribute;
		GLuint ssbo = 0;

	private:
		int texWidth;
		int texHeight;
		int comp;
		int pixels;
		int gridWidth;
		int gridHeight; //It says gridHeight it acualy is the gridLength
		int gridSize;
		int IBOCounter;
		float startingPosX;
		float startingPosZ;

		//Heightmap data storage array
		Vertex* heightMap;

		//Heightmap texture
		GLuint height_texture;

		//Smooth normals
		//glm::vec3 tmpBufferArray[66049];

		//Moved here from Render
		//Grass and Road
		int grassHeight;
		int grassWidth;
		int roadHeight;
		int roadWidth;
		GLuint grass_texture;
		GLuint road_texture;
		//Blend map
		int blendMapHeight; //It says gridHeight it acualy is the gridLength
		int blendMapWidth;
		GLuint blend_texture;
		float* y;
};

#endif