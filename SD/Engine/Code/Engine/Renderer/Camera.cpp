#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"

void Camera::SetOrthoView(Vec2 const& bottomLeft, Vec2 const& topRight)
{
	m_mode = CameraMode::ORTHOGRAPHIC;
	m_bottomLeft = bottomLeft;
	m_topRight = topRight;
}


void Camera::SetViewport(AABB2 const& viewport)
{
	m_viewport = viewport;
}


void Camera::SetPerspectiveView(float aspect, float fov, float near, float far)
{
	m_mode = CameraMode::PERSPECTIVE;
	m_aspect = aspect;
	m_fov = fov;
	m_near = near;
	m_far = far;
}


void Camera::SetFOV(float fov)
{
	m_fov = fov;
}


Vec2 Camera::GetOrthoBottomLeft() const
{
	return m_bottomLeft;
}


Vec2 Camera::GetOrthoTopRight() const
{
	return m_topRight;
}


Vec2 Camera::GetOrthoSize() const
{
	return m_topRight - m_bottomLeft;
}


Vec2 Camera::GetOrthoHalfSize() const
{
	return 0.5f * GetOrthoSize();
}


AABB2 Camera::GetOrthoDimensions() const
{
	return AABB2(m_bottomLeft, m_topRight);
}


AABB2 Camera::GetViewport() const
{
	return m_viewport;
}


void Camera::Translate(Vec2 const& offset)
{
	m_bottomLeft += offset;
	m_topRight += offset;
}


EulerAngles Camera::GetViewOrientation() const
{
	return m_viewOrientation;
}


Mat44 Camera::GetOrthoMatrix() const
{
	return Mat44::CreateOrthoProjection(m_bottomLeft.x, m_topRight.x, m_bottomLeft.y, m_topRight.y, 0.f, 1.f);
}


Mat44 Camera::GetPerspectiveMatrix() const
{
	return Mat44::CreatePerspectiveProjection(m_fov, m_aspect, m_near, m_far);
}


Mat44 Camera::GetProjectionMatrix() const
{
	Mat44 projection;
	switch (m_mode)
	{
		case CameraMode::ORTHOGRAPHIC:	
			projection = GetOrthoMatrix();
			break;
		case CameraMode::PERSPECTIVE:	
			projection = GetPerspectiveMatrix();
			break;
		default:						
			ERROR_AND_DIE("Error, you shouldn't be here");
			break;
	}

	Mat44 renderMatrix = GetRenderMatrix().GetOrthonormalInverse();
	projection.Append(renderMatrix);
	return projection;
}


Mat44 Camera::GetViewMatrix() const
{
	Mat44 view;
	view.SetTranslation3D(m_viewPosition);
	view.Append(m_viewOrientation.GetAsMatrix_XFwd_YLeft_ZUp());
	return view.GetOrthonormalInverse();
}


Mat44 Camera::GetRenderMatrix() const
{
	return m_renderMatrix;
}


void Camera::SetTransform(Vec3 const& viewPosition, EulerAngles const& viewOrientation)
{
	m_viewPosition = viewPosition;
	m_viewOrientation = viewOrientation;
}


void Camera::SetRenderTransform(Vec3 const& iBasis, Vec3 const& jBasis, Vec3 const& kBasis)
{
	m_renderMatrix = Mat44(iBasis, jBasis, kBasis, Vec3::ZERO);
}


Vec3 Camera::GetCameraPosition() const
{
	return m_viewPosition;
}


Vec3 Camera::GetCameraForward() const
{
	return m_viewOrientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
}


float Camera::GetCameraFOV() const
{
	return m_fov;
}


void Camera::SetColorTarget(Texture* color)
{
	m_colorTarget = color;
}


void Camera::SetDepthTarget(Texture* depth)
{
	m_depthTarget = depth;
}


Texture* Camera::GetColorTarget() const
{
	return m_colorTarget;
}


Texture* Camera::GetDepthTarget() const
{
	return m_depthTarget;
}
