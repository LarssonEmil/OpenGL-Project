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
		float nx, ny, nz, d;
	};
	struct Plane2
	{
		glm::vec3 pos;
		glm::vec3 dir;
	};
	Plane p_planes[6];
	Plane2 p_planes2[6];
	
	QuadTree(GLuint* dataStruct, int dimentions);
	~QuadTree();
	void Build(GLuint* dataStruct, Node* _this, int x, int y, int newSize, int endSplit);
	void Colapse(Node* _this, int level);
	Node* root;
	void ExtractPlanes(glm::mat4* comboMatrix, bool normaliz);
	void Draw(Node* _this, int depth, glm::mat4* viewMat);
	bool CullingAABB(Node* _this);
	bool CullingSQUARE(Node* _this, glm::mat4* viewMat);

};

#endif