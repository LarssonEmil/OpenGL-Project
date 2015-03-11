#ifndef QUADTREE_H
#define QUADTREE_H

#include <gl/glew.h>
#include <gl/GL.h>

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

	QuadTree(GLuint* dataStruct, int dimentions);
	void Build(GLuint* dataStruct, Node* _this, int x, int y, int newSize, int endSplit);
	Node* root;
	void Draw(Node* _this, Plane* frustumPlanes, int depth);
};

#endif