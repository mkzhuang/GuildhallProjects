#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/D3D11Common.hpp"

TextureView::TextureView()
{
	m_view = nullptr;
}


TextureView::~TextureView()
{
	DX_SAFE_RELEASE(m_sourceHandle);
	DX_SAFE_RELEASE(m_view);
}


