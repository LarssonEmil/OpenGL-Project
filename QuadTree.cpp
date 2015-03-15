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

void QuadTree::Draw(Node* _this, int depth, glm::vec3 CamPos)
{
	bool resultInside = true;
	int counter = 0;
	
	const glm::vec3 aabbCenter = glm::vec3(_this->bounds.center[0], 0, _this->bounds.center[1]);
	const glm::vec3 aabbSize = glm::vec3(_this->bounds.halfDimension, 0, _this->bounds.halfDimension);
	
	for (unsigned int iPlane = 0; iPlane < 6; ++iPlane)
	{
		const Plane& fP = p_planes[iPlane];
		
		float d = aabbCenter.x * fP.nx +
			aabbCenter.y * fP.ny +
			aabbCenter.z * fP.nz;

		float r = aabbSize.x * fP.nx +
			aabbSize.y * fP.ny +
			aabbSize.z * fP.nz;

		float r2 = 0 * fP.nx +
			aabbSize.y * fP.ny +
			aabbSize.z * fP.nz;

		float d_p_r = d + r;
		float d_m_r = d - r;

		float d_p_r2 = d + r2;
		float d_m_r2 = d - r2;

		int check = 0;
		if (d_p_r < -fP.d) //1,1 corner
		{
			check++;
		}
		if (d_m_r < -fP.d) //0,0 corner
		{
			check++;
		}

		if (d_p_r2 < -fP.d) //0,1 corner
		{
			check++;
		}
		if (d_m_r2 < -fP.d) //1,0 corner
		{
			check++;
		}

		if (check == 4)
		{
			counter++;
			break;
		}
		
	}
	if (counter > 0)
		resultInside = false;

	if (resultInside)
	{
		if (depth == 0)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *_this->data);
			glDrawElements(GL_TRIANGLE_STRIP, 152, GL_UNSIGNED_INT, 0);
		}
		else
		{
			depth--;
			Draw(_this->NE, depth, CamPos);
			Draw(_this->SE, depth, CamPos);
			Draw(_this->SW, depth, CamPos);
			Draw(_this->NW, depth, CamPos);
		}
	}
}
