#pragma once
#include "Engine/GUI/UIElement.hpp"

class Texture;

enum {TopLeft, TopCenter, TopRight, MiddleLeft, MiddleCenter, MiddleRight, BottomLeft, BottomCenter, BottomRight};

class UIImage : public UIElement
{
public:
	UIImage();
	UIImage(std::string const& name, AABB2 const& bound, UIAnchor const& anchor, Renderer* renderer, Texture* texture = nullptr, bool isStretch = false, AABB2 const& textureUV = AABB2::ZERO_TO_ONE, bool isDraggable = false, bool hasBorder = false);
	~UIImage();

	void Update() override;
	void Render() const override;
	void OnClick() override;

	void SetTexture(Texture* texture);

public:
	Texture* m_texture = nullptr;
	AABB2 m_textureUV = AABB2::ZERO_TO_ONE;
	bool m_isStretch = false;
	bool m_hasBorder = false;
};