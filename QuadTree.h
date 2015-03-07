#ifndef QUADTREE_H
#define QUADTREE_H

#include <gl/glew.h>
#include <gl/GL.h>

class QuadTree()
{
private:
	struct AABB()
	{
		float center = float[3];
		float halfDimension;
	};
	struct Node()
	{
		AABB bounds;
		Node* NE;
		Node* SE;
		Node* SW;
		Node* NW;
		GLuint* data;
	};
public:
	QuadTree();
	Node* root;
};

#endif