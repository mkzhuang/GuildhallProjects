#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/D3D11Common.hpp"

IndexBuffer::IndexBuffer(size_t size)
	: m_size(size)
{
}

IndexBuffer::~IndexBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
