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

		void loadImage();  //Loads the image we want to use.
		void createTexture( GLubyte* image );  //Makes the image/preHeightMap to a texture.
		void createRealHeightMap(); //Creates the actual heightmap
		void createIBO();  //Creates a buffer that holds in which order the vertices should be rendered.
		int getIBOCount();
		int getGridSize();
		int getGridWidth();
		int getGridHeight();
		float getStartingX();
		float getStartingZ();
		GLuint getTexture();
		
		Vertex* getRealHeightMap();

		GLuint gIndexBuffer;
		GLuint *subIndexBuffers;
		glm::vec2 cameraUV;
		float mat1Scale;
		float mat2Scale;
		GLuint gIndexAttribute;
		GLuint gHeightMapBuffer;
		GLuint gHeightMapAttribute;
		GLuint ssbo = 0;

	private:
		int texWidth;
		int texHeight;
		int comp;
		int pixels;
		int gridWidth;
		int gridHeight;
		int gridSize;
		int IBOCounter;
		float startingPosX;
		float startingPosZ;

		float* preHeightMap;
		Vertex* heightMap;
		GLuint height_texture;

		//Moved here from Render
		//Grass and Road
		int grassHeight;
		int grassWidth;
		int roadHeight;
		int roadWidth;
		GLuint grass_texture;
		GLuint road_texture;
		//Blend map
		int blendMapHeight;
		int blendMapWidth;
		GLuint blend_texture;
		float* y;
		

};

#endif