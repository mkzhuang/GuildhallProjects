#pragma once
#include "Engine/Math/IntVec2.hpp"

#include <string>
#include <vector>

struct ID3D11Texture2D;
class TextureView;

enum TextureBindFlagBit : unsigned int
{
	TEXTURE_BIND_NONE = 0,
	TEXTURE_BIND_SHADER_RESOURCE_BIT = (1 << 0),
	TEXTURE_BIND_RENDER_TARGET_BIT = (1 << 1),
	TEXTURE_BIND_DEPTH_STENCIL_BIT = (1 << 2),
};
typedef unsigned int TextureBindFlags;

enum class TextureFormat : int
{
	R8G8B8A8_UNORM,
	D24_UNORM_S8_UINT,
	R24G8_TYPELESS,
};

enum class MemoryHint : int
{
	STATIC,
	GPU,
	DYNAMIC,
};


struct TextureCreateInfo
{
	std::string name = "";
	IntVec2 dimensions = IntVec2::ZERO;
	TextureFormat format = TextureFormat::R8G8B8A8_UNORM;
	TextureBindFlags bindFlags = TEXTURE_BIND_SHADER_RESOURCE_BIT;
	MemoryHint memoryHint = MemoryHint::STATIC;

	void const* initialData = nullptr;
	size_t initialDataByteSize = 0;

	ID3D11Texture2D* handle = nullptr;
};


struct TextureViewInfo
{
	TextureBindFlagBit type = TextureBindFlagBit::TEXTURE_BIND_NONE;

	bool operator== (TextureViewInfo const& other) const
	{
		return type == other.type;
	};
};

class Texture
{
	friend class Renderer;

public:
	Texture() {}
	~Texture();
	Texture(Texture const& copy);
	
public:
	IntVec2 GetDimensions() const {return m_dimensions;}
	std::string const& GetImageFilePath() const {return m_name;}

	TextureView* GetShaderResourceView();
	TextureView* GetDepthStencilView();
	TextureView* GetRenderTargetView();
	TextureView* FindOrCreateView(TextureViewInfo const& info);

protected:
	Renderer* m_owner = nullptr;
	std::string m_name;
	IntVec2 m_dimensions;
	ID3D11Texture2D* m_texture = nullptr;
	TextureFormat m_format = TextureFormat::R8G8B8A8_UNORM;
	TextureBindFlags m_allowedBinds = TEXTURE_BIND_NONE;
	std::vector<TextureView*> m_textureViews;
};


