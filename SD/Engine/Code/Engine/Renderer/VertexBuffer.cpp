#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

VertexBuffer::VertexBuffer(size_t size, unsigned int stride)
	: m_size(size)
	, m_stride(stride)
{
}


VertexBuffer::~VertexBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}


unsigned int VertexBuffer::GetStride() const
{
	return m_stride;
}


