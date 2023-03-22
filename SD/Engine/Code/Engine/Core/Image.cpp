#include "Engine/Core/Image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb/stb_image.h"


Image::Image(char const* imageFilePath)
	:m_imageFilePath(imageFilePath)
{
	int width, height, channels;
	stbi_set_flip_vertically_on_load(1);
	unsigned char *colors = stbi_load(imageFilePath, &width, &height, &channels, 0);
	m_dimensions = IntVec2(width, height);
	int size = width * height;
	for (int colorIndex = 0; colorIndex < size; colorIndex++)
	{
		if (channels == 3)
		{
			unsigned char r = colors[colorIndex * 3];
			unsigned char g = colors[colorIndex * 3 + 1];
			unsigned char b = colors[colorIndex * 3 + 2];
			unsigned char a = 255;
			m_rgbaTexels.emplace_back(r, g, b, a);
		}
		else if (channels == 4)
		{
			unsigned char r = colors[colorIndex * 4];
			unsigned char g = colors[colorIndex * 4 + 1];
			unsigned char b = colors[colorIndex * 4 + 2];
			unsigned char a = colors[colorIndex * 4 + 3];
			m_rgbaTexels.emplace_back(r, g, b, a);
		}
	}

	free(colors);
}


Image::Image(IntVec2 size, Rgba8 color)
{
	m_dimensions = size;
	int pixelSize = (size.x * size.y);
	for (int index = 0; index < pixelSize; index++)
	{
		m_rgbaTexels.push_back(Rgba8(color.a, color.g, color.b, color.a));
	}
}


std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}


IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}


const void* Image::GetRawData() const
{
	return m_rgbaTexels.data();
}


Rgba8 Image::GetTexelColor(IntVec2 const& texelCoords) const
{
	int index = GetIndexFromCoordinates(texelCoords);
	return m_rgbaTexels[index];
}


void Image::SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor)
{
	int index = GetIndexFromCoordinates(texelCoords);
	m_rgbaTexels[index] = newColor;
}


int Image::GetIndexFromCoordinates(IntVec2 const& texelCoords) const
{
	return texelCoords.x + texelCoords.y * m_dimensions.x;
}


