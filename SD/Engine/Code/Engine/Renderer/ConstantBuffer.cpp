#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/D3D11Common.hpp"

ConstantBuffer::ConstantBuffer(size_t size)
	: m_size(size)
{
}


ConstantBuffer::~ConstantBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}


