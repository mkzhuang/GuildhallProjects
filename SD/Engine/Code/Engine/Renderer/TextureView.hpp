#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Texture.hpp"

#include <string>

class Renderer;
class Texture;

struct TextureViewInfo;
struct ID3D11Texture2D;
struct ID3D11View;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;

class TextureView
{
friend class Renderer;
friend class Texture;

private:
	TextureView();
	~TextureView();

	TextureView(TextureView const&) = delete;

private:
	Texture* m_source = nullptr;
	ID3D11Texture2D* m_sourceHandle = nullptr;
	TextureViewInfo m_info;

	union 
	{
		ID3D11View* m_view;
		ID3D11ShaderResourceView* m_shaderResourceView;
		ID3D11RenderTargetView* m_renderTargetView;
		ID3D11DepthStencilView* m_depthStencilView;
	};
};