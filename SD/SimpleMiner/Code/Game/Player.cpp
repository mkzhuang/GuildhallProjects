#include "Game/Player.hpp"
#include "Game//GameCommon.hpp"
#include "Game/Chunk.hpp"
#include "Game/BlockIterator.hpp"
#include "Game/BlockDefinition.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/EventSystem.hpp"

constexpr float PITCH_MAX = 89.9f;
constexpr float ROLL_MAX = 45.f;
constexpr float FIX_ANGLE_DISTANCE = 10.f;
constexpr float OVER_SHOULDER_DISTANCE = 4.f;
constexpr int PREVENTATIVE_RAYS = 12;

Player::Player(World* owner, Vec3 const& startPos, EulerAngles const& orientation, Camera* camera)
	: Entity(owner, startPos, orientation)
	, m_camera(camera)
{
	m_definition = EntityDefinition::GetByName("player");
	m_bounds = m_definition->m_bounds;
	m_speed = m_definition->m_speed;
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasicSprites_64x64_CrackIncluded.png");
	SpriteSheet spriteSheet(*m_texture, IntVec2(64, 64));
	for (int uvIndex = 0; uvIndex < 6; uvIndex++)
	{
		m_crackUVs.push_back(spriteSheet.GetSpriteUVs(IntVec2(32 + uvIndex, 46)));
	}
	m_borderUVs = spriteSheet.GetSpriteUVs(IntVec2(5, 33));

	m_holdingBlocks[0] = BlockDefinition::GetIndexByName("grass");
	m_holdingBlocks[1] = BlockDefinition::GetIndexByName("dirt");
	m_holdingBlocks[2] = BlockDefinition::GetIndexByName("plank");
	m_holdingBlocks[3] = BlockDefinition::GetIndexByName("stone");
	m_holdingBlocks[4] = BlockDefinition::GetIndexByName("bricks");
	m_holdingBlocks[5] = BlockDefinition::GetIndexByName("glowstone");
	m_holdingBlocks[6] = BlockDefinition::GetIndexByName("sand");
	m_holdingBlocks[7] = BlockDefinition::GetIndexByName("oak_log");
}


Player::~Player()
{
}


void Player::Update(float deltaSeconds)
{
	if (g_theInput->WasKeyJustPressed('H'))
	{
		m_orientationDegree = EulerAngles(0.f, 0.f, 0.f);
		m_position = Vec3(0.f, 0.f, 90.f);
	}

	UpdateMovement(deltaSeconds);

	switch (m_cameraMode)
	{
		case GameCameraMode::FIRST_PERSON:			UpdateFirstPerson();				break;
		case GameCameraMode::FIXED_ANGLE_TRACKING:	UpdateFixedAngleTracking();			break;
		case GameCameraMode::OVER_SHOULDER:			UpdateOverShoulder();				break;
		case GameCameraMode::SPECTATOR:				UpdateSpectator(deltaSeconds);		break;
		case GameCameraMode::INDEPENDENT:			UpdateIndependent(deltaSeconds);	break;
	}

	UpdateMode();
	UpdateHoldingBlock();
	UpdateRaycast(deltaSeconds);
}


void Player::Render() const
{
	std::vector<Vertex_PCU> raycastVerts;
	if (m_cameraMode != GameCameraMode::FIRST_PERSON)
	{
		float maxLength = m_raycastResult.m_maxDistance;
		Vec3 const& rayStart = m_raycastResult.m_startPosition;
		Vec3 const& forwardNormal = m_raycastResult.m_forwardNormal;
		if (m_raycastResult.m_didImpact)
		{
			Vec3 const& impactPos = m_raycastResult.m_impactPosition;
			AddVertsForLine3D(raycastVerts, rayStart, rayStart + forwardNormal * maxLength, 0.01f, Rgba8::GRAY);
			AddVertsForLine3D(raycastVerts, rayStart, impactPos, 0.01f, Rgba8::RED);
		}
		else
		{
			AddVertsForLine3D(raycastVerts, rayStart, rayStart + forwardNormal * maxLength, 0.01f, Rgba8::GREEN);
		}

		g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		g_theRenderer->SetModelColor(Rgba8::WHITE);
		g_theRenderer->SetModelMatrix(Mat44());
		g_theRenderer->BindTexture(nullptr);
		std::vector<Vertex_PCU> boundsVerts;
		AddVertsForWireAABB3D(boundsVerts, m_bounds, 0.01f, Rgba8::CYAN);
		Vec3 bottomCornerA = m_bounds.m_mins;
		Vec3 bottomCornerB = Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, m_bounds.m_mins.z);
		Vec3 bottomCornerC = Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, m_bounds.m_mins.z);
		Vec3 bottomCornerD = Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, m_bounds.m_mins.z);
		AddVertsForLine3D(boundsVerts, bottomCornerA, bottomCornerD, 0.005f, Rgba8::CYAN);
		AddVertsForLine3D(boundsVerts, bottomCornerB, bottomCornerC, 0.005f, Rgba8::CYAN);
		g_theRenderer->DrawVertexArray(boundsVerts);
	}

	if (m_raycastResult.m_didImpact)
	{
		Vec3 const& impactPos = m_raycastResult.m_impactPosition;
		Vec3 const& impactSurfaceNormal = m_raycastResult.m_impactSurfaceNormal;
		BlockIterator blockHit = m_raycastResult.m_impactBlock;
		AABB3 blockBounds = blockHit.GetBlockBounds();
		AddVertsForArrow3D(raycastVerts, impactPos, impactPos + impactSurfaceNormal * 0.2f, 0.01f, Rgba8::BLUE);
		AddVertsForSphere3D(raycastVerts, impactPos, 0.03f, 16.f, 8.f);
		AddVertsForWireAABB3D(raycastVerts, blockBounds, 0.01f, Rgba8::MAGENTA);

		g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
		g_theRenderer->SetModelColor(Rgba8::WHITE);
		g_theRenderer->SetModelMatrix(Mat44());
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->DrawVertexArray(raycastVerts);
	}

	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(m_crackVerts);

	if (!g_isDebugging) return;

	std::vector<Vertex_PCU> basisVerts;
	AddVertsForBasis3D(basisVerts, 0.05f);
	Vec3 cameraForward = m_camera->GetCameraForward();
	AddVertsForScreenBasis3D(basisVerts, m_camera->GetCameraPosition() + cameraForward * 0.2f, 0.01f, 0.0003f);
	g_theRenderer->SetDepthStencilState(DepthTest::LESSEQUAL, true);
	g_theRenderer->SetModelColor(Rgba8::WHITE);
	g_theRenderer->SetModelMatrix(Mat44());
	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(basisVerts);
}


void Player::RenderUI(Camera camera) const
{
	std::vector<Vertex_PCU> inventoryVerts;

	AABB2 cameraDimensions = camera.GetOrthoDimensions();
	float distancePerBox = INVENTORY_BOX_SIZE + 20.f;
	float beginX = 0.5f * cameraDimensions.m_maxs.x - (static_cast<float>(INVENTORY_SLOTS / 2) * distancePerBox) - (0.5f * INVENTORY_BOX_SIZE);
	float inventoryY = 10.f;
	for (int boxIndex = 0; boxIndex < INVENTORY_SLOTS; boxIndex++)
	{
		float boxMinX = beginX + static_cast<float>(boxIndex) * distancePerBox;
		Vec2 boxMin(boxMinX, inventoryY);
		Vec2 boxMax(boxMinX + INVENTORY_BOX_SIZE, inventoryY + INVENTORY_BOX_SIZE);
		AABB2 box = AABB2(boxMin, boxMax);
		AABB2 borderBox = AABB2(boxMin - Vec2(5.0f, 5.0f), boxMax + Vec2(5.0f, 5.0f));
		Rgba8 color;
		if (boxIndex == m_holdingBlockIndex) color = Rgba8::RED;
		AABB2 blockDefSideUVs = BlockDefinition::GetById(m_holdingBlocks[boxIndex])->m_sideUVs;
		AddVertsForAABB2D(inventoryVerts, borderBox, color, m_borderUVs.m_mins, m_borderUVs.m_maxs);
		AddVertsForAABB2D(inventoryVerts, box, Rgba8::WHITE, blockDefSideUVs.m_mins, blockDefSideUVs.m_maxs);
	}

	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray(inventoryVerts);
}


void Player::UpdateMode()
{
	if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
	{
		m_cameraMode = (GameCameraMode)((uint8_t)m_cameraMode + 1);
		if (m_cameraMode == GameCameraMode::NUM_MODES)
		{
			m_cameraMode = GameCameraMode::FIRST_PERSON;
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_physicsMode = (GamePhysicsMode)((uint8_t)m_physicsMode + 1);
		if (m_physicsMode == GamePhysicsMode::NUM_MODES)
		{
			m_physicsMode = GamePhysicsMode::WALKING;
		}
	}
}


void Player::UpdateHoldingBlock()
{
	MouseWheelState state = g_theInput->GetMouseWheelState();
	switch (state)
	{
		case MouseWheelState::WHEEL_DOWN:
			m_holdingBlockIndex++;
			if (m_holdingBlockIndex == INVENTORY_SLOTS)
			{
				m_holdingBlockIndex = 0;
			}
			break;
		case MouseWheelState::WHEEL_IDLE:
			break;
		case MouseWheelState::WHEEL_UP:
			m_holdingBlockIndex--;
			if (m_holdingBlockIndex == -1)
			{
				m_holdingBlockIndex = INVENTORY_SLOTS - 1;
			}
			break;
	}
}


void Player::UpdateRaycast(float deltaSeconds)
{
	m_crackVerts.clear();
	m_referencePos = GetEyePosition();
	m_referenceForward = GetForwardNormal();
	if (m_world) 
	{
		m_raycastResult = m_world->RaycastVsBlocks(m_referencePos, m_referenceForward, MAX_RAYCAST_LENGTH);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_MIDDLE_MOUSE) && m_world)
	{
		if (m_raycastResult.m_didImpact)
		{
			BlockIterator blockItr = m_raycastResult.m_impactBlock;
			uint8_t blockType = blockItr.GetBlock()->m_type;
			bool found = false;
			for (int inventoryIndex = 0; inventoryIndex < INVENTORY_SLOTS; inventoryIndex++)
			{
				if (m_holdingBlocks[inventoryIndex] == blockType)
				{
					m_holdingBlockIndex = inventoryIndex;
					found = true;
					break;
				}
			}

			if (!found)
			{
				int firstAirIndex = -1;
				for (int inventoryIndex = 0; inventoryIndex < INVENTORY_SLOTS; inventoryIndex++)
				{
					if (m_holdingBlocks[inventoryIndex] == BlockDefinition::GetIndexByName("air"))
					{
						m_holdingBlockIndex = inventoryIndex;
						firstAirIndex = inventoryIndex;
						m_holdingBlocks[inventoryIndex] = blockType;
						break;
					}
				}

				if (firstAirIndex == -1)
				{
					m_holdingBlocks[m_holdingBlockIndex] = blockType;
				}
			}
		}
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE) && m_world)
	{
		if (m_raycastResult.m_didImpact)
		{
			BlockIterator blockItr = m_raycastResult.m_impactBlock;

			if (blockItr == m_blockLastFrame)
			{
				m_diggingTimer += deltaSeconds;
				float blockBreakTimer = BlockDefinition::GetById(blockItr.GetBlock()->m_type)->m_breakTime;
				if (m_diggingTimer >= blockBreakTimer)
				{
					uint8_t blockType = blockItr.GetBlock()->m_type;
					blockItr.m_chunk->DigBlockAt(blockItr);
					m_diggingTimer = 0.f;
					bool found = false;
					for (int inventoryIndex = 0; inventoryIndex < INVENTORY_SLOTS; inventoryIndex++)
					{
						if (m_holdingBlocks[inventoryIndex] == blockType)
						{
							found = true;
							break;
						}
					}

					if (!found)
					{
						int firstAirIndex = -1;
						for (int inventoryIndex = 0; inventoryIndex < INVENTORY_SLOTS; inventoryIndex++)
						{
							if (m_holdingBlocks[inventoryIndex] == BlockDefinition::GetIndexByName("air"))
							{
								firstAirIndex = inventoryIndex;
								m_holdingBlocks[inventoryIndex] = blockType;
								break;
							}
						}

						if (firstAirIndex == -1)
						{
							m_holdingBlocks[m_holdingBlockIndex] = blockType;
						}
					}
				}
				else
				{
					int crackFrame = RoundDownToInt(RangeMap(m_diggingTimer, 0.f, blockBreakTimer, 0.f, 5.99f));
					IntVec3 pos = blockItr.m_chunk->GetWorldCoordsForIndex(blockItr.m_blockIndex);
					Vec3 mins = Vec3(pos) - Vec3(0.001f, 0.001f, 0.001f);
					Vec3 maxs = Vec3(pos + IntVec3::ONE) + Vec3(0.001f, 0.001f, 0.001f);
					AddVertsForAABB3D(m_crackVerts, AABB3(mins, maxs), Rgba8::WHITE, m_crackUVs[crackFrame]);
				}
			}
			else
			{
				m_diggingTimer = 0.f;
			}
		}
		else
		{
			m_diggingTimer = 0.f;
		}	
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE) && m_world)
	{
		if (m_raycastResult.m_didImpact)
		{
			BlockIterator blockItr = m_raycastResult.m_impactBlock;
			if (m_raycastResult.m_impactSurfaceNormal == Vec3::EAST)
			{
				blockItr = blockItr.GetEastNeighbor();
			}
			else if (m_raycastResult.m_impactSurfaceNormal == Vec3::WEST)
			{
				blockItr = blockItr.GetWestNeighbor();
			}
			else if (m_raycastResult.m_impactSurfaceNormal == Vec3::NORTH)
			{
				blockItr = blockItr.GetNorthNeighbor();
			}
			else if (m_raycastResult.m_impactSurfaceNormal == Vec3::SOUTH)
			{
				blockItr = blockItr.GetSouthNeighbor();
			}
			else if (m_raycastResult.m_impactSurfaceNormal == Vec3::UP)
			{
				blockItr = blockItr.GetTopNeighbor();
			}
			else if (m_raycastResult.m_impactSurfaceNormal == Vec3::DOWN)
			{
				blockItr = blockItr.GetBottomNeighbor();
			}
			blockItr.m_chunk->PlaceBlockAt(blockItr, m_holdingBlocks[m_holdingBlockIndex]);
		}
	}
	m_blockLastFrame = m_raycastResult.m_impactBlock;
}


void Player::UpdateFirstPerson()
{
	m_camera->SetTransform(GetEyePosition(), m_orientationDegree);
}


void Player::UpdateFixedAngleTracking()
{
	Vec3 eyePos = GetEyePosition();
	EulerAngles orientation(40.f, 30.f, 0.f);
	Vec3 cameraForward = orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetIBasis3D();
	GameRaycastResult3D raycast = m_world->RaycastVsBlocks(eyePos, -cameraForward, FIX_ANGLE_DISTANCE);
	float distance = raycast.m_didImpact ? raycast.m_impactDistance * 0.9f : FIX_ANGLE_DISTANCE;
	Vec3 cameraPos = eyePos + cameraForward * -distance;
	m_camera->SetTransform(cameraPos, orientation);
}


void Player::UpdateOverShoulder()
{
	Vec3 eyePos = GetEyePosition();
	Vec3 cameraForward = GetForwardNormal();
	GameRaycastResult3D raycast = m_world->RaycastVsBlocks(eyePos, -cameraForward, OVER_SHOULDER_DISTANCE);
	float distance = raycast.m_didImpact ? raycast.m_impactDistance * 0.9f : OVER_SHOULDER_DISTANCE;
	Vec3 cameraPos = eyePos + cameraForward * -distance;
	m_camera->SetTransform(cameraPos, m_orientationDegree);
}


void Player::UpdateSpectator(float deltaSeconds)
{
	Mat44 orientationMatrix = m_camera->GetViewOrientation().GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 iBasis = orientationMatrix.GetIBasis3D();
	Vec3 jBasis = orientationMatrix.GetJBasis3D();

	iBasis.z = 0.f;
	jBasis.z = 0.f;
	iBasis = iBasis.GetNormalized();
	jBasis = jBasis.GetNormalized();

	Vec3 velocity = Vec3::ZERO;
	float speed = m_speed;
	if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
	{
		speed *= m_sprintMultiplier;
	}

	if (g_theInput->IsKeyDown('W'))
	{
		velocity += iBasis * speed;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		velocity -= iBasis * speed;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		velocity += jBasis * speed;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		velocity -= jBasis * speed;
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		velocity.z += speed;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		velocity.z -= speed;
	}

	Vec2 mouseMovement = g_theInput->GetMouseClientDelta();
	mouseMovement *= (m_mouseScale);

	Vec3 cameraPos = m_camera->GetCameraPosition() + velocity * deltaSeconds;
	EulerAngles cameraAngle = m_camera->GetViewOrientation() + EulerAngles(mouseMovement.x, -mouseMovement.y, 0.f);
	cameraAngle.m_pitchDegrees = Clamp(cameraAngle.m_pitchDegrees, -PITCH_MAX, PITCH_MAX);
	cameraAngle.m_rollDegrees = Clamp(cameraAngle.m_rollDegrees, -ROLL_MAX, ROLL_MAX);

	m_camera->SetTransform(cameraPos, cameraAngle);
}


void Player::UpdateIndependent(float deltaSeconds)
{
	UNUSED(deltaSeconds)
}


void Player::UpdateMovement(float deltaSeconds)
{
	Mat44 orientationMatrix = m_orientationDegree.GetAsMatrix_XFwd_YLeft_ZUp();
	Vec3 iBasis = orientationMatrix.GetIBasis3D();
	Vec3 jBasis = orientationMatrix.GetJBasis3D();

	iBasis.z = 0.f;
	jBasis.z = 0.f;
	iBasis = iBasis.GetNormalized();
	jBasis = jBasis.GetNormalized();

	float speed = m_speed;
	if (m_cameraMode != GameCameraMode::SPECTATOR)
	{
		if (g_theInput->IsKeyDown(KEYCODE_SHIFT))
		{
			speed *= m_sprintMultiplier;
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

		if (m_physicsMode != GamePhysicsMode::WALKING)
		{
			m_acceleration *= m_flyingMultiplier;
			if (g_theInput->IsKeyDown('Q'))
			{
				m_acceleration.z += speed * m_flyingMultiplier * m_flyingZMultiplier;
			}
			if (g_theInput->IsKeyDown('E'))
			{
				m_acceleration.z -= speed * m_flyingMultiplier * m_flyingZMultiplier;
			}
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) && m_velocity.z == 0.f)
		{
			AddImpulse(Vec3(0.f, 0.f, 12.5f));
		}

		Vec2 mouseMovement = g_theInput->GetMouseClientDelta();
		mouseMovement *= (m_mouseScale);
		m_orientationDegree += EulerAngles(mouseMovement.x, -mouseMovement.y, 0.f);

		float& pitch = m_orientationDegree.m_pitchDegrees;
		pitch = Clamp(pitch, -PITCH_MAX, PITCH_MAX);

		float& roll = m_orientationDegree.m_rollDegrees;
		roll = Clamp(roll, -ROLL_MAX, ROLL_MAX);
	}
	
	Vec3 negativeDragVelocity = -m_velocity;
	if (m_physicsMode == GamePhysicsMode::WALKING)
	{
		negativeDragVelocity.z = 0.f;
		AddForce(Vec3(0.f, 0.f, GRAVITY));
	}
	AddForce(m_definition->m_drag * negativeDragVelocity);
	m_velocity += m_acceleration * deltaSeconds;

	float remainingDeltaSeconds = deltaSeconds;
	if (m_physicsMode != GamePhysicsMode::NOCLIP) remainingDeltaSeconds = UpdatePreventative(deltaSeconds);

	m_position += m_velocity * remainingDeltaSeconds;
	m_acceleration = Vec3::ZERO;
	m_bounds.SetCenter(m_position);
}


float Player::UpdatePreventative(float deltaSeconds)
{
	while (deltaSeconds > 0.f)
	{
		if (m_velocity == Vec3::ZERO) return deltaSeconds;

		Vec3 corners[PREVENTATIVE_RAYS];
		m_bounds.GetCorners(corners);
		Vec3 boundsHalfDimension = m_bounds.GetHalfDimensions();
		Vec3 boundsCenter = m_bounds.GetCenter();
		corners[8] = boundsCenter + Vec3(-boundsHalfDimension.x, -boundsHalfDimension.y, 0.f);
		corners[9] = boundsCenter + Vec3(boundsHalfDimension.x, -boundsHalfDimension.y, 0.f);
		corners[10] = boundsCenter + Vec3(-boundsHalfDimension.x, boundsHalfDimension.y, 0.f);
		corners[11] = boundsCenter + Vec3(boundsHalfDimension.x, boundsHalfDimension.y, 0.f);

		float rayLength = (m_velocity * deltaSeconds).GetLength();
		Vec3 rayForward = m_velocity.GetNormalized();
		GameRaycastResult3D shortestHit;
		shortestHit.m_impactDistance = 999.f;
		for (int rayIndex = 0; rayIndex < PREVENTATIVE_RAYS; rayIndex++)
		{
			Vec3 starPos = corners[rayIndex];
			GameRaycastResult3D raycast = m_world->RaycastVsBlocks(corners[rayIndex], rayForward, rayLength);
			if (raycast.m_didImpact && raycast.m_impactDistance < shortestHit.m_impactDistance && raycast.m_impactSurfaceNormal != -rayForward)
			{
				shortestHit = raycast;
			}
		}

		if (shortestHit.m_didImpact)
		{
			float length = shortestHit.m_impactDistance;
			Vec3 impactSurfaceNormal = shortestHit.m_impactSurfaceNormal;
			float impactDeltaSeconds = (length / rayLength) * deltaSeconds;
			m_position += m_velocity * impactDeltaSeconds;
			m_bounds.SetCenter(m_position);
			if (impactSurfaceNormal.x != 0.f)
			{
				m_velocity.x = 0.f;
			}
			if (impactSurfaceNormal.y != 0.f)
			{
				m_velocity.y = 0.f;
			}
			if (impactSurfaceNormal.z != 0.f)
			{
				m_velocity.z = 0.f;
			}
			deltaSeconds -= impactDeltaSeconds;
		}
		else
		{
			return deltaSeconds;
		}
	}

	return deltaSeconds;
}



