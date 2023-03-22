#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/TextureView.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"


static DXGI_FORMAT GetD3D11Format(TextureFormat format);

Texture::Texture(Texture const& copy)
	: m_name(copy.m_name)
	, m_dimensions(copy.m_dimensions)
{

}


Texture::~Texture()
{
	DX_SAFE_RELEASE(m_texture);
	for (TextureView* view : m_textureViews)
	{
		delete view;
	}
	m_textureViews.clear();
}


TextureView* Texture::GetShaderResourceView()
{
	TextureViewInfo info;
	info.type = TEXTURE_BIND_SHADER_RESOURCE_BIT;

	return FindOrCreateView(info);
}


TextureView* Texture::GetDepthStencilView()
{
	TextureViewInfo info;
	info.type = TEXTURE_BIND_DEPTH_STENCIL_BIT;

	return FindOrCreateView(info);
}


TextureView* Texture::GetRenderTargetView()
{
	TextureViewInfo info;
	info.type = TEXTURE_BIND_RENDER_TARGET_BIT;

	return FindOrCreateView(info);
}


TextureView* Texture::FindOrCreateView(TextureViewInfo const& info)
{
	for (TextureView* view : m_textureViews)
	{
		if (view->m_info == info)
		{
			return view;
		}
	}

	TextureView* view = new TextureView();
	view->m_sourceHandle = nullptr;
	view->m_source = this;
	m_texture->AddRef();
	view->m_sourceHandle = m_texture;
	view->m_info = info;

	ID3D11Device* device;
	m_texture->GetDevice(&device);

	switch (info.type)
	{
		case TEXTURE_BIND_SHADER_RESOURCE_BIT:
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceDesc = {};
				shaderResourceDesc.Format = GetD3D11Format(m_format);
				shaderResourceDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				shaderResourceDesc.Texture2D.MostDetailedMip = 0;
				shaderResourceDesc.Texture2D.MipLevels = 1;
				device->CreateShaderResourceView(m_texture, &shaderResourceDesc, &view->m_shaderResourceView);
			}
			break;
		case TEXTURE_BIND_DEPTH_STENCIL_BIT:
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
				depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
				depthStencilDesc.Flags = 0;
				depthStencilDesc.Texture2D.MipSlice = 0;
				device->CreateDepthStencilView(m_texture, &depthStencilDesc, &view->m_depthStencilView);
			}
			break;
		case TEXTURE_BIND_RENDER_TARGET_BIT:
			device->CreateRenderTargetView(m_texture, nullptr, &view->m_renderTargetView);
			break;
		default:
			DX_SAFE_RELEASE(device);
			ERROR_AND_DIE("invalid view type.");
			
	}
	DX_SAFE_RELEASE(device);

	m_textureViews.push_back(view);
	return view;
}


DXGI_FORMAT GetD3D11Format(TextureFormat format)
{
	switch (format)
	{
		case TextureFormat::R8G8B8A8_UNORM:		return DXGI_FORMAT_R8G8B8A8_UNORM;
		case TextureFormat::D24_UNORM_S8_UINT:	return DXGI_FORMAT_D24_UNORM_S8_UINT;
		case TextureFormat::R24G8_TYPELESS:		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		default:								ERROR_AND_DIE("unsupport format");
	}
}


