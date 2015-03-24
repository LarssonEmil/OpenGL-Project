#include "QuadTree.h"
using namespace glm;

QuadTree::QuadTree(GLuint* dataStruct, int dimentions)
{
	root = new Node();
	Build(dataStruct, root, 0, 0, dimentions, 5);

	//near
	p_planes2[0].pos = { 0, 0, 0 };
	p_planes2[0].dir = { 0, 0, 1 };
	//right
	p_planes2[1].pos = { 0, 0, 0 };
	p_planes2[1].dir = { 0.50, 0, 0.50 };
	//left
	p_planes2[2].pos = { 0, 0, 0 };
	p_planes2[2].dir = { -0.50, 0, 0.50 };
	//bot
	p_planes2[3].pos = { 0, 0, 0 };
	p_planes2[3].dir = { 0, -0.55, 0.45 };
	//top
	p_planes2[4].pos = { 0, 0, 0 };
	p_planes2[4].dir = { 0, 0.55, 0.45 };
	//far
	p_planes2[5].pos = { 0, 0, 500 };
	p_planes2[5].dir = { 0, 0, -1 };
}

QuadTree::~QuadTree()
{
	Colapse(root, 5);
	delete root;
}

void QuadTree::Colapse(Node* _this, int level)
{
	if (level == 0)
	{
		return;
	}
	level -= 1;
	Colapse(_this->NE, level);
	delete _this->NE;
	Colapse(_this->SE, level);
	delete _this->SE;
	Colapse(_this->SW, level);
	delete _this->SW;
	Colapse(_this->NW, level);
	delete _this->NW;
	return;
}

void QuadTree::Build(GLuint* dataStruct, Node* _this, int edgeX, int edgeY, int newSize, int endSplit)
{
	int half = newSize / 2;
	_this->bounds.center[0] = (float)(edgeX + half);
	_this->bounds.center[1] = (float)(edgeY + half);
	_this->bounds.halfDimension = (float)half;

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

void QuadTree::ExtractPlanes(glm::mat4* comboMatri, bool normaliz)
{
	glm::mat4 comboMatrix = *comboMatri;
	// Left clipping plane 
	p_planes[0].nx = comboMatrix[0].w + comboMatrix[0].x;
	p_planes[0].ny = comboMatrix[1].w + comboMatrix[1].x;
	p_planes[0].nz = comboMatrix[2].w + comboMatrix[2].x;
	p_planes[0].d = comboMatrix[3].w + comboMatrix[3].x;
	// Right clipping plane	
	p_planes[1].nx = comboMatrix[0].w - comboMatrix[0].x;
	p_planes[1].ny = comboMatrix[1].w - comboMatrix[1].x;
	p_planes[1].nz = comboMatrix[2].w - comboMatrix[2].x;
	p_planes[1].d = comboMatrix[3].w - comboMatrix[3].x;
	// Top clipping plane	
	p_planes[2].nx = comboMatrix[0].w - comboMatrix[0].y;
	p_planes[2].ny = comboMatrix[1].w - comboMatrix[1].y;
	p_planes[2].nz = comboMatrix[2].w - comboMatrix[2].y;
	p_planes[2].d = comboMatrix[3].w - comboMatrix[3].y;
	// Bottom clipping plane
	p_planes[3].nx = comboMatrix[0].w + comboMatrix[0].y;
	p_planes[3].ny = comboMatrix[1].w + comboMatrix[1].y;
	p_planes[3].nz = comboMatrix[2].w + comboMatrix[2].y;
	p_planes[3].d = comboMatrix[3].w + comboMatrix[3].y;
	// Near clipping plane
	p_planes[4].nx = comboMatrix[0].w + comboMatrix[0].z;
	p_planes[4].ny = comboMatrix[1].w + comboMatrix[1].z;
	p_planes[4].nz = comboMatrix[2].w + comboMatrix[2].z;
	p_planes[4].d = comboMatrix[3].w + comboMatrix[3].z;
	// Far clipping plane	
	p_planes[5].nx = comboMatrix[0].w - comboMatrix[0].z;
	p_planes[5].ny = comboMatrix[1].w - comboMatrix[1].z;
	p_planes[5].nz = comboMatrix[2].w - comboMatrix[2].z;
	p_planes[5].d = comboMatrix[3].w - comboMatrix[3].z;
	
	// Normalize the plane equations, if requested
	if (normaliz)
	{
		//p_planes[0] = glm::normalize(p_planes[0]);
		//p_planes[1] = glm::normalize(p_planes[1]);
		//p_planes[2] = glm::normalize(p_planes[2]);
		//p_planes[3] = glm::normalize(p_planes[3]);
		//p_planes[4] = glm::normalize(p_planes[4]);
		//p_planes[5] = glm::normalize(p_planes[5]);
	}	
}

void QuadTree::Draw(Node* _this, int depth, glm::mat4* viewMat)
{
	bool resultInside = false;
	int counter = 0;
	
	if (depth == 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *_this->data);
		glDrawElements(GL_TRIANGLE_STRIP, 152, GL_UNSIGNED_INT, 0);
	}
	else
	{
		bool draw = true;
		//draw = CullingAABB(_this); //fast
		//draw = CullingSQUARE(_this, viewMat); //slow
		if (!draw)
			return;
		depth--;
		Draw(_this->NE, depth, viewMat);
		Draw(_this->SE, depth, viewMat);
		Draw(_this->SW, depth, viewMat);
		Draw(_this->NW, depth, viewMat);
	}
}

bool QuadTree::CullingAABB(Node* _this)
{
	int check00 = 0;
	int check11 = 0;
	int check01 = 0;
	int check10 = 0;

	const glm::vec3 aabbCenter = glm::vec3(_this->bounds.center[0], 0, _this->bounds.center[1]);
	const glm::vec3 aabbSize = glm::vec3(_this->bounds.halfDimension, 0, _this->bounds.halfDimension);

	for (int n = 0; n < 6; n++)
	{
		Plane fP = p_planes[n];
		
		float d = aabbCenter.x * fP.nx +
			aabbCenter.y * fP.ny +
			aabbCenter.z * fP.nz;

		float r = aabbSize.x * fP.nx +
			aabbSize.y * fP.ny +
			aabbSize.z * fP.nz;

	//	float r2 = 0 * fP.nx +
	//		aabbSize.y * fP.ny +
	//		aabbSize.z * fP.nz;
	//
	//	float r3 = aabbSize.x * fP.nx +
	//		aabbSize.y * fP.ny +
	//		0 * fP.nz;

		float d_p_r = d + r;
		float d_m_r = d - r;

	//	float d_p_r2 = d + r2;
	//	float d_m_r2 = d + r3;

		if (d_p_r < -fP.d) //1,1 corner
			check11++;
		if(d_m_r < -fP.d) //0,0 corner
			check00++; 
	//	if (d_p_r2 < -fP.d) //0,1 corner
	//		check01++;
	//	if (d_m_r2 < -fP.d) //1,0 corner
	//		check10++;

	}
	if (check11 == 0) //corners are inside
		return true;
	if (check00 == 0)
		return true;
	//if (check01 == 0)
	//	return true;
	//if (check10 == 0)
	//	return true;

	return false;
}

bool QuadTree::CullingSQUARE(Node* _this, glm::mat4* viewMat)
{
	
	const glm::vec3 aabbCenter = glm::vec3(_this->bounds.center[0], 0, _this->bounds.center[1]);
	const glm::vec3 aabbSize = glm::vec3(_this->bounds.halfDimension, 0, _this->bounds.halfDimension);

	glm::vec3 corners[4];
	corners[0] = glm::vec3(aabbCenter.x - aabbSize.x, 0, aabbCenter.z - aabbSize.z);
	corners[1] = glm::vec3(aabbCenter.x - aabbSize.x, 0, aabbCenter.z + aabbSize.z);
	corners[2] = glm::vec3(aabbCenter.x + aabbSize.x, 0, aabbCenter.z - aabbSize.z);
	corners[3] = glm::vec3(aabbCenter.x + aabbSize.x, 0, aabbCenter.z + aabbSize.z);
	
	//corners
	for(unsigned int n = 0; n < 4; ++n)
	{
		int counter = 0;
	    //planes
		for (int p = 0; p < 5; p++)
		{
			glm::vec3 pp = glm::vec3(*viewMat * glm::vec4(corners[n], 1));
			vec3 planeToPoint = pp - p_planes2[p].pos;
			if (dot(pp, p_planes2[p].dir) < 0)
				counter++; //inside n planes
		}
		if (counter == 5)
		{
			return true;
		}	
	}
	return false;
}