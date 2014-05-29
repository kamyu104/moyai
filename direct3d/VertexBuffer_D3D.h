#pragma once

#include "../common/VertexFormat.h"
#include "../common.h"
#include "Context_D3D.h"

class FragmentShader_D3D;

class VertexBuffer_D3D 
{

public:

	VertexBuffer_D3D(VertexFormat *format, unsigned int maxVertexCount, const FragmentShader_D3D *inputSignatureShader);
	~VertexBuffer_D3D(); 

	void copyFromBuffer(void *data, int vert_cnt);
	void setCoord(unsigned int index, Vec3 v );
	Vec3 getCoord(unsigned index);
	void setCoordBulk(Vec3 *v, unsigned int num);
	void setColor(unsigned int index, Color c);
	Color getColor(unsigned int index);
	void setUV(unsigned int index, Vec2 uv);
	Vec2 getUV(unsigned int index);
	void setUVBulk(Vec2 *uv, unsigned int num);
	void setNormal(unsigned int index, Vec3 v);
	Vec3 getNormal(unsigned int index);
	void setNormalBulk(Vec3 *v, unsigned int num);
	Vec3 calcCenterOfCoords();
	void dump();

	void copyToGPU();
	void bind();
	void setTopology(D3D11_PRIMITIVE_TOPOLOGY topology);

private:

	void initD3DObjects();
	void setPositionElement(D3D11_INPUT_ELEMENT_DESC &element);
	void setUVElement(D3D11_INPUT_ELEMENT_DESC &element);
	void setNormalElement(D3D11_INPUT_ELEMENT_DESC &element);
	void setColorElement(D3D11_INPUT_ELEMENT_DESC &element);

	ID3D11Buffer *m_pVertexBuffer;
	ID3D11InputLayout *m_pInputLayout;
	const FragmentShader_D3D *m_pInputSignatureShader;
	D3D11_PRIMITIVE_TOPOLOGY m_topology;

	VertexFormat *m_pVertexFormat;
	void *m_pBuffer;
	UINT m_vertexStride;
	UINT m_vertexCount;
	UINT m_bufferSize;
};