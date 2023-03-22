#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/AABB2.hpp"

class Texture;

enum class CameraMode
{
	ORTHOGRAPHIC,
	PERSPECTIVE
};

class Camera
{
public:
	void SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight);
	void SetViewport(AABB2 const& viewport);
	void SetPerspectiveView(float aspect, float fov, float near, float far);
	void SetFOV(float fov);
	Vec2 GetOrthoBottomLeft() const;
	Vec2 GetOrthoTopRight() const;
	Vec2 GetOrthoSize() const;
	Vec2 GetOrthoHalfSize() const;
	AABB2 GetOrthoDimensions() const;
	AABB2 GetViewport() const;
	void Translate(Vec2 const& offset);
	EulerAngles GetViewOrientation() const;
	Mat44 GetOrthoMatrix() const;
	Mat44 GetPerspectiveMatrix() const;
	Mat44 GetProjectionMatrix() const;
	Mat44 GetViewMatrix() const;
	Mat44 GetRenderMatrix() const;
	void SetTransform(Vec3 const& viewPosition, EulerAngles const& viewOrientation);
	void SetRenderTransform(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis);
	Vec3 GetCameraPosition() const;
	Vec3 GetCameraForward() const;
	float GetCameraFOV() const;

	void SetColorTarget(Texture* color);
	void SetDepthTarget(Texture* depth);
	Texture* GetColorTarget() const;
	Texture* GetDepthTarget() const;

private:
	Vec2 m_bottomLeft;
	Vec2 m_topRight;
	AABB2 m_viewport = AABB2::ZERO_TO_ONE;
	float m_aspect = 0.f;
	float m_fov = 0.f;
	float m_near = 0.f;
	float m_far = 0.f;
	CameraMode m_mode = CameraMode::ORTHOGRAPHIC;
	Vec3 m_viewPosition;
	EulerAngles m_viewOrientation;
	Mat44 m_renderMatrix;

	Texture* m_colorTarget = nullptr;
	Texture* m_depthTarget = nullptr;
};


