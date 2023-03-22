#include "Game/Player.hpp"
#include "Game/Game.hpp"
#include "Game/Actor.hpp"
#include "Game/ActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Window/Window.hpp"

constexpr float PITCH_MAX = 85.f;
constexpr float ROLL_MAX = 45.f;
constexpr float COUNTDOWN_TIMER = 5.f;

Player::Player(Game* game, Vec3 const& pos, Camera* camera, Camera* UICamera)
	: m_game(game)
	, m_position(pos)
	, m_camera(camera)
	, m_UICamera(UICamera)
{
	m_camera->SetRenderTransform(Vec3(0.f, -1.f, 0.f), Vec3(0.f, 0.f, 1.f), Vec3(1.f, 0.f, 0.f));
}


Player::~Player()
{
}


void Player::Update(float deltaSeconds)
{
	if (!GetActor() || GetActor()->m_isDead)
	{
		return;
	}

	if (g_theInput->WasKeyJustPressed('F'))
	{
		if ((int)m_game->GetAllPlayer().size() >= 2) return;
		m_freeFlyCameraMode = !m_freeFlyCameraMode;
	}

	if (m_freeFlyCameraMode)
	{
		FreeFlyUpdate(deltaSeconds);
	}
	else
	{
		ActorUpdate(deltaSeconds);
	}

	m_animationTimer += deltaSeconds;

	WeaponDefinition const* currentWeaponDef = GetActor()->GetEquippedWeapon()->m_definition;
	if (m_isAttacking && m_animationTimer >= currentWeaponDef->m_refireTime)
	{
		m_isAttacking = false;
		m_animationTimer = 0.f;
	}

	Mat44 orientation = GetActor()->m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	g_theAudio->UpdateListener(m_playerIndex, m_position, orientation.GetIBasis3D(), orientation.GetKBasis3D());
	Vec2 const& viewportDimensions = m_camera->GetViewport().GetDimensions();
	float viewportAspect = viewportDimensions.x / viewportDimensions.y;
	Actor* currentActor = GetActor();
	float fov = currentActor ? currentActor->m_definition->m_cameraFOVDegrees : 60.f;
	m_camera->SetPerspectiveView(g_theWindow->GetAspect() * viewportAspect, fov, 0.1f, 100.f);
}


void Player::ActorUpdate(float deltaSeconds)
{
	if (m_controllerIndex == -1)
	{
		KeyboardUpdate(deltaSeconds);
	}
	else
	{
		ControllerUpdate(deltaSeconds);
	}
}


void Player::FreeFlyUpdate(float deltaSeconds)
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

	// controller movement
	Vec2 controllerMovement = controller.GetLeftStick().GetPosition();
	m_velocity += jBasis * (-controllerMovement.x * speed);
	m_velocity += iBasis * (controllerMovement.y * speed);

	// controller orientation
	Vec2 controllerOrientation = deltaSeconds == 0.f ? Vec2(0.f, 0.f) : controller.GetRightStick().GetPosition();
	m_orientationDegree += EulerAngles(-controllerOrientation.x, -controllerOrientation.y, 0.f) * m_mouseScale * 0.5f;
	m_orientationDegree.m_rollDegrees += deltaSeconds == 0.f ? 0.f : (controller.GetRightTrigger() - controller.GetLeftTrigger());

	if (g_theInput->IsKeyDown('W'))
	{
		m_velocity += iBasis * speed;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_velocity -= iBasis * speed;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_velocity += jBasis * speed;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_velocity -= jBasis * speed;
	}
	if (g_theInput->IsKeyDown('Z') || controller.IsButtonDown(XboxButtonID::LEFT_SHOULDER))
	{
		m_velocity.z += speed;
	}
	if (g_theInput->IsKeyDown('C') || controller.IsButtonDown(XboxButtonID::RIGHT_SHOULDER))
	{
		m_velocity.z -= speed;
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		m_orientationDegree.m_rollDegrees -= deltaSeconds == 0.f ? 0.f : m_speed;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		m_orientationDegree.m_rollDegrees += deltaSeconds == 0.f ? 0.f : m_speed;
	}
	m_position += m_velocity * deltaSeconds;

	Vec2 mouseMovement = g_theInput->GetMouseClientDelta();
	mouseMovement *= (m_mouseScale * deltaSeconds);
	m_orientationDegree += EulerAngles(mouseMovement.x, -mouseMovement.y, 0.f);

	float& pitch = m_orientationDegree.m_pitchDegrees;
	pitch = Clamp(pitch, -PITCH_MAX, PITCH_MAX);

	float& roll = m_orientationDegree.m_rollDegrees;
	roll = Clamp(roll, -ROLL_MAX, ROLL_MAX);
	m_camera->SetTransform(m_position, m_orientationDegree);
}


void Player::KeyboardUpdate(float deltaSeconds)
{
	Actor* curActor = GetActor();
	if (!curActor) return;

	m_position = curActor->GetEyePosition();
	m_orientationDegree = curActor->m_orientation;
	m_camera->SetTransform(m_position, m_orientationDegree);

	Mat44 orientationMatrix = m_orientationDegree.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 iBasis = orientationMatrix.GetIBasis3D();
	Vec3 jBasis = orientationMatrix.GetJBasis3D();

	iBasis = Vec3(iBasis.x, iBasis.y, 0.f).GetNormalized();
	jBasis = Vec3(jBasis.x, jBasis.y, 0.f).GetNormalized();

	m_acceleration = Vec3::ZERO;
	float speed = curActor->m_definition->m_walkSpeed;
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		speed = curActor->m_definition->m_runSpeed;
	}

	if (g_theInput->IsKeyDown('W'))
	{
		m_acceleration += iBasis * speed;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_acceleration -= iBasis * speed;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_acceleration += jBasis * speed;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_acceleration -= jBasis * speed;
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		m_orientationDegree.m_rollDegrees -= deltaSeconds == 0.f ? 0.f : m_speed;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		m_orientationDegree.m_rollDegrees += deltaSeconds == 0.f ? 0.f : m_speed;
	}

	if (g_theInput->IsKeyDown('1'))
	{
		curActor->EquipWeapon(0);
		m_isAttacking = false;
		m_animationTimer = 0.f;
	}

	if (g_theInput->IsKeyDown('2'))
	{
		curActor->EquipWeapon(1);
		m_isAttacking = false;
		m_animationTimer = 0.f;
	}

	if (g_theInput->IsKeyDown('3'))
	{
		curActor->EquipWeapon(2);
		m_isAttacking = false;
		m_animationTimer = 0.f;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		curActor->Attack();
		if (!m_isAttacking)
		{
			m_isAttacking = true;
			m_animationTimer = 0.f;
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFTARROW))
	{
		curActor->EquipPreviousWeapon();
		m_isAttacking = false;
		m_animationTimer = 0.f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHTARROW))
	{
		curActor->EquipNextWeapon();
		m_isAttacking = false;
		m_animationTimer = 0.f;
	}

	curActor->AddForce(curActor->m_definition->m_drag * m_acceleration);

	Vec2 mouseMovement = g_theInput->GetMouseClientDelta();
	mouseMovement *= (m_mouseScale * deltaSeconds);
	m_orientationDegree += EulerAngles(mouseMovement.x, -mouseMovement.y, 0.f);

	float& pitch = m_orientationDegree.m_pitchDegrees;
	pitch = Clamp(pitch, -PITCH_MAX, PITCH_MAX);

	float& roll = m_orientationDegree.m_rollDegrees;
	roll = Clamp(roll, -ROLL_MAX, ROLL_MAX);
	curActor->m_orientation = m_orientationDegree;
}


void Player::ControllerUpdate(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0);

	Actor* curActor = GetActor();
	if (!curActor) return;

	m_position = curActor->GetEyePosition();
	m_orientationDegree = curActor->m_orientation;
	m_camera->SetTransform(m_position, m_orientationDegree);

	Mat44 orientationMatrix = m_orientationDegree.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 iBasis = orientationMatrix.GetIBasis3D();
	Vec3 jBasis = orientationMatrix.GetJBasis3D();

	iBasis = Vec3(iBasis.x, iBasis.y, 0.f).GetNormalized();
	jBasis = Vec3(jBasis.x, jBasis.y, 0.f).GetNormalized();

	m_acceleration = Vec3::ZERO;
	float speed = curActor->m_definition->m_walkSpeed;
	if (controller.IsButtonDown(XboxButtonID::LEFT_SHOULDER))
	{
		speed = curActor->m_definition->m_runSpeed;
	}

	// controller movement
	Vec2 controllerMovement = controller.GetLeftStick().GetPosition();
	m_acceleration += jBasis * (-controllerMovement.x * speed);
	m_acceleration += iBasis * (controllerMovement.y * speed);

	// controller orientation
	Vec2 controllerOrientation = deltaSeconds == 0.f ? Vec2(0.f, 0.f) : controller.GetRightStick().GetPosition();
	m_orientationDegree += EulerAngles(-controllerOrientation.x, -controllerOrientation.y, 0.f) * m_mouseScale * 0.5f;

	if (controller.IsButtonDown(XboxButtonID::RIGHT_SHOULDER))
	{
		curActor->Attack();
		if (!m_isAttacking)
		{
			m_isAttacking = true;
			m_animationTimer = 0.f;
		}
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_X))
	{
		curActor->EquipPreviousWeapon();
		m_isAttacking = false;
		m_animationTimer = 0.f;
	}

	if (controller.WasButtonJustPressed(XboxButtonID::BUTTON_Y))
	{
		curActor->EquipNextWeapon();
		m_isAttacking = false;
		m_animationTimer = 0.f;
	}

	curActor->AddForce(curActor->m_definition->m_drag * m_acceleration);

	float& pitch = m_orientationDegree.m_pitchDegrees;
	pitch = Clamp(pitch, -PITCH_MAX, PITCH_MAX);

	float& roll = m_orientationDegree.m_rollDegrees;
	roll = Clamp(roll, -ROLL_MAX, ROLL_MAX);
	curActor->m_orientation = m_orientationDegree;
}


void Player::Render() const
{	
	g_theRenderer->BeginCamera(*m_UICamera);

	Vec2 const& UIBottomLeft = m_UICamera->GetOrthoBottomLeft();
	Vec2 const& UITopRight = m_UICamera->GetOrthoTopRight();
	AABB2 uiBounds(Vec2(0.f, 0.15f * UITopRight.y), UITopRight);
	BitmapFont* font = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/SquirrelFixedFont");
	if (!GetActor())
	{
		std::vector<Vertex_PCU> verts;
		std::string diedText = Stringf("You Died");
		float diedTextLength = static_cast<float>(diedText.size());
		AABB2 diedTextBounds(Vec2::ZERO, Vec2(100.f * diedTextLength, 100.f));
		uiBounds.AlignBoxWithin(diedTextBounds, Vec2(0.5f, 0.5f));
		font->AddVertsForTextInBox2D(verts, diedTextBounds, 100.f, diedText, Rgba8::RED);
		g_theRenderer->BindTexture(&font->GetTexture());
		g_theRenderer->DrawVertexArray(verts);
		return;
	}

	WeaponDefinition const* currentWeaponDef = GetActor()->GetEquippedWeapon()->m_definition;
	if (!currentWeaponDef) return;

	g_theRenderer->BindShader(currentWeaponDef->m_shader);
	std::vector<Vertex_PCU> hudVerts;
	AABB2 hudBounds(UIBottomLeft, Vec2(UITopRight.x, 0.15f * UITopRight.y));
	AddVertsForAABB2D(hudVerts, hudBounds);
	g_theRenderer->BindTexture(currentWeaponDef->m_hubBaseTexture);
	g_theRenderer->DrawVertexArray(hudVerts);

	std::vector<Vertex_PCU> reticleVerts;
	Vec2 UICameraCenter = 0.5f * (UIBottomLeft + UITopRight);
	Vec2 reticleHalfDimensions = 0.5f * currentWeaponDef->m_reticeSize;
	AABB2 reticleBounds(UICameraCenter - reticleHalfDimensions, UICameraCenter + reticleHalfDimensions);
	AddVertsForAABB2D(reticleVerts, reticleBounds);
	g_theRenderer->BindTexture(currentWeaponDef->m_reticleTexture);
	g_theRenderer->DrawVertexArray(reticleVerts);

	std::vector<Vertex_PCU> weaponVerts;
	AABB2 spriteBounds(Vec2::ZERO, currentWeaponDef->m_spriteSize);
	uiBounds.AlignBoxWithin(spriteBounds, currentWeaponDef->m_spritePivot);
	if (m_isAttacking)
	{
		SpriteDefinition const& spriteDef = currentWeaponDef->m_attackAnimationDefinition->GetSpriteDefAtTime(m_animationTimer);
		AABB2 uvs = spriteDef.GetUVs();
		AddVertsForAABB2D(weaponVerts, spriteBounds, Rgba8::WHITE, uvs.m_mins, uvs.m_maxs);
		g_theRenderer->BindTexture(&spriteDef.GetTexture());
	}
	else
	{
		SpriteDefinition const& spriteDef = currentWeaponDef->m_idleAnimationDefinition->GetSpriteDefAtTime(m_animationTimer);
		AABB2 uvs = spriteDef.GetUVs();
		AddVertsForAABB2D(weaponVerts, spriteBounds, Rgba8::WHITE, uvs.m_mins, uvs.m_maxs);
		g_theRenderer->BindTexture(&spriteDef.GetTexture());
	}
	g_theRenderer->DrawVertexArray(weaponVerts);

	std::vector<Vertex_PCU> infoVerts;
	std::string health = Stringf("%.0f", GetActor()->m_health);
	AABB2 healthBounds(Vec2::ZERO, Vec2(180.f, 50.f));
	hudBounds.AlignBoxWithin(healthBounds, Vec2(0.2f, 0.55f));
	font->AddVertsForTextInBox2D(infoVerts, healthBounds, 50.f, health);

	std::string kills = Stringf("%i", m_kills);
	AABB2 killsBounds(Vec2::ZERO, Vec2(180.f, 50.f));
	hudBounds.AlignBoxWithin(killsBounds, Vec2(0.375f, 0.55f));
	font->AddVertsForTextInBox2D(infoVerts, killsBounds, 50.f, kills);

	if (!m_map->m_waveTimer.HasDurationElapsed())
	{
		int waveCount = m_map->m_waveCounter + 1;
		int remainingTime = static_cast<int>(COUNTDOWN_TIMER - static_cast<float>(m_map->m_waveTimer.GetElapsedTime()));
		std::string timer = Stringf("Wave %i in %i seconds", waveCount, remainingTime);
		float timerTextLength = static_cast<float>(timer.size());
		AABB2 timerBounds(Vec2::ZERO, Vec2(60.f * timerTextLength, 50.f));
		uiBounds.AlignBoxWithin(timerBounds, Vec2(0.5f, 0.5f));
		font->AddVertsForTextInBox2D(infoVerts, timerBounds, 50.f, timer, Rgba8::GREEN);
	}
	g_theRenderer->BindTexture(&font->GetTexture());
	g_theRenderer->DrawVertexArray(infoVerts);

	if (GetActor()->m_isDead)
	{
		std::vector<Vertex_PCU> pauseVertexArray;
		AABB2 screenBox(m_UICamera->GetOrthoBottomLeft(), m_UICamera->GetOrthoTopRight());
		AddVertsForAABB2D(pauseVertexArray, screenBox, Rgba8(150, 150, 150, 150));
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(int(pauseVertexArray.size()), pauseVertexArray.data());
	}
	g_theRenderer->EndCamera(*m_UICamera);
}


Mat44 Player::GetModelMatrix() const
{
	Mat44 model;
	model.SetTranslation3D(m_position);
	Mat44 rotation = m_orientationDegree.GetAsMatrix_XFwd_YLeft_ZUp();
	model.Append(rotation);
	return model;
}


