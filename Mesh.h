#pragma once

#include <d3d11.h>
#include "Vertex.h"

class Mesh {
public:
	Mesh(Vertex* vertices, int numVertex, unsigned int* indices, int numIndex, ID3D11Device *device);
	Mesh(const char* objFile, ID3D11Device *device);
	~Mesh();
	
	ID3D11Buffer *GetVertexBuffer();
	ID3D11Buffer *GetIndexBuffer();
	int GetIndexCount();
	

private:

	ID3D11Buffer *vertexBufferMesh;
	ID3D11Buffer *indexBufferMesh;
	//ID3D11Device *deviceMesh;
	int indices1;

	void CreateBuffers(Vertex *vertices, int numVertex, unsigned int *indices, int numIndex, ID3D11Device *device);
	
};

