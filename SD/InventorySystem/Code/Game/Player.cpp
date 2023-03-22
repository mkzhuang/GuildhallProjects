#include "Game/Player.hpp"
#include "Game//GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"

constexpr float PITCH_MAX = 85.f;
constexpr float ROLL_MAX = 45.f;

Player::Player(Game* owner, Vec3 const& startPos)
	: Entity(owner, startPos)
{
	m_bounds = AABB3(Vec3::ZERO, Vec3(1.f, 1.f, 1.8f));
	m_bounds.SetCenter(Vec3(0.f, 0.f, 0.9f));
	AddVertsForWireAABB3D(m_verts, m_bounds, 0.05f, Rgba8::CYAN);
}


Player::~Player()
{
}


void Player::Update(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0);

	Mat44 orientationMatrix = m_orientationDegree.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 iBasis = orientationMatrix.GetIBasis3D();
	Vec3 jBasis = orientationMatrix.GetJBasis3D();

	m_velocity = Vec3::ZERO;
	float speed = m_speed;
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT) || controller.IsButtonDown(XboxButtonID::BUTTON_A))
	{
		speed *= m_sprintMultiplier;
	}

	if (g_theInput->IsKeyDown('W'))
	{
		m_velocity += jBasis;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_velocity -= jBasis;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_velocity -= iBasis;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_velocity += iBasis;
	}
	m_velocity = m_velocity.GetNormalized() * speed;
	m_position += m_velocity * deltaSeconds;
}


void Player::Render() const
{
	Mat44 model = GetModelMatrix();
	g_theRenderer->SetModelMatrix(model);
	g_theRenderer->DrawVertexArray(m_verts);
}


void Player::KeepInBound(AABB3 const& bounds)
{
	Vec3 halfDimensions = m_bounds.GetHalfDimensions();
	if (m_position.x - halfDimensions.x < bounds.m_mins.x)
	{
		m_position.x = bounds.m_mins.x + halfDimensions.x;
	}
	if (m_position.x + halfDimensions.x > bounds.m_maxs.x)
	{
		m_position.x = bounds.m_maxs.x - halfDimensions.x;
	}
	if (m_position.y - halfDimensions.y < bounds.m_mins.y)
	{
		m_position.y = bounds.m_mins.y + halfDimensions.y;
	}
	if (m_position.y + halfDimensions.y > bounds.m_maxs.y)
	{
		m_position.y = bounds.m_maxs.y - halfDimensions.y;
	}
}


