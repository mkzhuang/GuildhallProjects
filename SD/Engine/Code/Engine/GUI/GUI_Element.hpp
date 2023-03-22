#pragma once
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

#include <string>
#include <vector>

class GUI_Canvas;
class Texture;
class Renderer;
class VertexBuffer;
class ConstantBuffer;

enum {TopLeft, TopCenter, TopRight, MiddleLeft, MiddleCenter, MiddleRight, BottomLeft, BottomCenter, BottomRight};

enum class GUI_AnchorType
{
	NONE,
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	MIDDLE_LEFT,
	CENTER,
	MIDDLE_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT,
	HORIONTAL_STRETCH,
	VERTICAL_STRETCH,
	FULL_STRETCH,
};

enum class GUI_BorderStyle
{
	NONE,
	ROUNDED_NONE,
	RECT,
	ROUNDED,
	DOUBLE_RECT,
	DOUBLE_ROUNDED,
	SINGLE_DIVISION,
	DOUBLE_DIVISION,
	DOUBLE_DIVISION_GAP,
};

enum class GUI_BackgroundStyle
{
	DEFAULT,
	NINE_SLICE,
};

enum class GUI_AnimationStyle
{
	NONE,
	UP_GLOW,
};

const int BORDER_CONSTANT_BUFFER_SLOT = 4;

struct BorderConstant
{
	Vec2 BoxMins = Vec2(0.f, 0.f);
	Vec2 BoxMaxs = Vec2(1.f, 1.f);
	float Radius = 1.f;
	Vec3 padding;
};

class GUI_Element
{
public:
	GUI_Element();
	virtual ~GUI_Element();

	virtual void Update();
	virtual void Render(Renderer* renderer) const;
	virtual void RenderHover(Renderer* renderer) const;
	virtual void RenderFocus(Renderer* renderer) const;
	virtual void RenderAnimation(Renderer* renderer) const;

	void SetCanvas(GUI_Canvas* canvas);
	void SetName(std::string const& name);
	void SetBounds(AABB2 const& bounds);
	void SetSize(Vec2 const& size);
	void SetAnchorType(GUI_AnchorType anchorType);
	void SetAnchorPosition(Vec2 const& position);
	void SetAnchorStretchOffset(Vec2 const& offset);
	//void SetPivot(Vec2 const& pivot);

	void SetIsDraggable(bool isDraggable);
	void SetDraggableAreaUVs(AABB2 const& uvs);

	void SetParent(GUI_Element* parent);
	void SetChildren(std::vector<GUI_Element*> children);
	void AddChild(GUI_Element* child);
	
	void SetBackgroundStyle(GUI_BackgroundStyle style);
	void SetSliceDepth(Vec2 const& sliceDepth);
	void SetBackgroundColor(Rgba8 const& color);
	void SetBackgroundUVs(AABB2 const& uvs);
	void SetBackgroundTexture(Texture* texture);
	void SetBorderStyle(GUI_BorderStyle style);
	void SetBorderColor(Rgba8 const& color);
	void SetBorderThickness(float thickness);
	void SetRoundBorderRadius(float radius);
	void SetBorderGap(Vec2 borderGap);
	void SetConstantBuffer();
	void BindConstantBuffer(Renderer* renderer);

	void SetCanHover(bool canHover);
	void SetIsHover(bool isHover);
	void SetHoverTexture(Texture* texture);
	void SetHoverColor(Rgba8 const& color);
	void SetIsFocus(bool isFocus);
	void SetFocusTexture(Texture* texture);
	void SetFocusColor(Rgba8 const& color);
	void SetAnimationTimer(float duration);
	void SetAnimationStyle(GUI_AnimationStyle style);

	void SetBufferDirty();
	bool IsPointInside(Vec2 const& mousePos) const;
	bool IsElementDraggable(Vec2 const& mousePos) const;

	void ReconstructElement();

private:
	void CalculateElementWindow();
	void CreateBorderVerts();
	void CreateBackgroundVerts();
	void CreateHoverAndFocusVerts();
	void CreateAnimationVerts();

public:
	// basic
	GUI_Canvas* m_canvas = nullptr;
	std::string m_name = "default";
	AABB2 m_elementWindow = AABB2::ZERO_TO_ONE;
	Vec2 m_size = Vec2::ZERO;
	Vec2 m_anchorPosition = Vec2::ZERO;
	GUI_AnchorType m_anchorType = GUI_AnchorType::CENTER;
	Vec2 m_anchorStretchOffset = Vec2::ZERO;
	Vec2 m_originPosition = Vec2::ZERO;
	//Vec2 m_pivot = Vec2::ZERO;

	// transform
	bool m_isDraggable = false;
	AABB2 m_draggingAreaUVs = AABB2::ZERO_TO_ONE;

	// hierarchy
	GUI_Element* m_parent = nullptr;
	std::vector<GUI_Element*> m_children;

	// stylize - border
	GUI_BackgroundStyle m_backgroundStyle = GUI_BackgroundStyle::DEFAULT;
	Vec2 m_sliceDepth = Vec2::ZERO;
	Rgba8 m_backgroundColor = Rgba8::CLEAR;
	AABB2 m_backgroundUVs = AABB2::ZERO_TO_ONE;
	Texture* m_backgroundTexture = nullptr;
	GUI_BorderStyle m_borderStyle = GUI_BorderStyle::NONE;
	Rgba8 m_borderColor = Rgba8::WHITE;
	float m_borderThickness = 0.1f;
	float m_roundBorderRadius = 1.f;
	Vec2 m_borderGap = Vec2::ZERO;
	BorderConstant m_borderConstant;
	ConstantBuffer* m_borderCBO = nullptr;

	// stylize - hover/focus
	bool m_canHover = true;
	bool m_isHover = false;
	Texture* m_hoverTexture = nullptr;
	Rgba8 m_hoverColor = Rgba8::CLEAR;
	bool m_isFocus = false;
	Texture* m_focusTexture = nullptr;
	Rgba8 m_focusColor = Rgba8::CLEAR;
	float m_animationDuration = 0.f;
	float m_animationTimer = 0.f;
	GUI_AnimationStyle m_animationStyle = GUI_AnimationStyle::NONE;

	// rendering
	std::vector<Vertex_PCU> m_borderVerts;
	std::vector<Vertex_PCU> m_backgroundVerts;
	std::vector<Vertex_PCU> m_hoverVerts;
	std::vector<Vertex_PCU> m_focusVerts;
	std::vector<Vertex_PCU> m_animationVerts;
	bool m_isBufferDirty = true;
};