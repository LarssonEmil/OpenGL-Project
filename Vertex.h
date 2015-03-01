#ifndef VERTEX_H
#define VERTEX_H


#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

struct Vertex
{
	float x, y, z; 
	float u, v;
	
	Vertex(){}

	Vertex( float nx, float ny, float nz, float nu, float nv )
	{
		x = nx;
		y = ny;
		z = nz;
		u = nu;
		v = nv;
	}
};

#endif