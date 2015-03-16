#ifndef QUADTREE_H
#define QUADTREE_H

#include <gl/glew.h>
#include <gl/GL.h>
#include <glm\glm.hpp>

class QuadTree
{
private:
	
public:
	struct AABB
	{
		float center[2];
		int halfDimension;
	};
	struct Node
	{
		AABB bounds;
		Node* NE;
		Node* SE;
		Node* SW;
		Node* NW;
		GLuint* data;
	};
	struct Plane
	{
//<<<<<<< HEAD
		//glm::vec3 pos, dir;
//=======
		glm::vec3 pos;
		glm::vec3 dir;
//>>>>>>> origin/master
	};
	Plane p_planes[6];
	
	QuadTree(GLuint* dataStruct, int dimentions);
	void Build(GLuint* dataStruct, Node* _this, int x, int y, int newSize, int endSplit);
	Node* root;
//<<<<<<< HEAD
	void Draw(Node* _this, Plane* frustumPlanes, int count, int depth);
//=======
	void ExtractPlanes(glm::mat4* comboMatrix, bool normaliz);
	void Draw(Node* _this, int depth, glm::mat4* viewMat);
//>>>>>>> origin/master
};

#endif