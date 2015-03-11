#include "QuadTree.h"

QuadTree::QuadTree(GLuint* dataStruct, int dimentions)
{
	root = new Node();
	Build(dataStruct, root, 0, 0, dimentions, 5);
}

void QuadTree::Build(GLuint* dataStruct, Node* _this, int edgeX, int edgeY, int newSize, int endSplit)
{
	int half = newSize / 2;
	_this->bounds.center[0] = edgeX + half;
	_this->bounds.center[1] = edgeY + half;
	_this->bounds.halfDimension = half;

	if (endSplit == 0)
	{
		int fetch = (edgeX) / 8;
		fetch += (edgeY / 8) * 32;
		_this->data = &dataStruct[fetch];
	}
	else
	{
		_this->data = 0;
		endSplit--;
		_this->NE = new Node();
		//1 0
		Build(dataStruct, _this->NE, _this->bounds.center[0], edgeY, half, endSplit);
		_this->SE = new Node();
		//1 1
		Build(dataStruct, _this->SE, _this->bounds.center[0], _this->bounds.center[1], half, endSplit);
		_this->SW = new Node();
		//0 1
		Build(dataStruct, _this->SW, edgeX, _this->bounds.center[1], half, endSplit);
		_this->NW = new Node();
		//0 0
		Build(dataStruct, _this->NW, edgeX, edgeY, half, endSplit);
	}
}

void QuadTree::Draw(Node* _this, int depth)
{
	if (depth == 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *_this->data);
		glDrawElements(GL_TRIANGLE_STRIP, 152, GL_UNSIGNED_INT, 0);
	}
	else
	{
		depth--;
		Draw(_this->NE, depth);
		Draw(_this->SE, depth);
		Draw(_this->SW, depth);
		Draw(_this->NW, depth);
	}
}
