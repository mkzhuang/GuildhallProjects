#pragma once
#include "Game/Entity.hpp"
#include "Game/World.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/Vertex_PCU.hpp"

constexpr int INVENTORY_SLOTS = 9;
constexpr float INVENTORY_BOX_SIZE = 95.f;

enum class GameCameraMode
{
	FIRST_PERSON,
	FIXED_ANGLE_TRACKING,
	OVER_SHOULDER,
	SPECTATOR,
	INDEPENDENT,

	NUM_MODES
};

class Player : public Entity
{
public:
	Player(World* owner, Vec3 const& startPos, EulerAngles const& orientation, Camera* camera);
	~Player();

	void Update(float deltaSeconds);
	void Render() const;
	void RenderUI(Camera camera) const;

private:
	void UpdateMode();
	void UpdateHoldingBlock();
	void UpdateRaycast(float deltaSeconds);
	void UpdateFirstPerson();
	void UpdateFixedAngleTracking();
	void UpdateOverShoulder();
	void UpdateSpectator(float deltaSeconds);
	void UpdateIndependent(float deltaSeconds);
	void UpdateMovement(float deltaSeconds);
	float UpdatePreventative(float deltaSeconds);

public:
	Camera* m_camera = nullptr;
	Texture const* m_texture = nullptr;
	float m_speed = 1.f;
	float m_mouseScale = .25f;
	float m_sprintMultiplier = 2.f;
	float m_flyingMultiplier = 10.0f;
	float m_flyingZMultiplier = 2.5f;
	Vec3 m_referencePos;
	Vec3 m_referenceForward;
	GameRaycastResult3D m_raycastResult;
	GameCameraMode m_cameraMode = GameCameraMode::FIRST_PERSON;

	float m_diggingTimer = 0.f;
	BlockIterator m_blockLastFrame;
	std::vector<AABB2> m_crackUVs;
	std::vector<Vertex_PCU> m_crackVerts;

	uint8_t m_holdingBlocks[INVENTORY_SLOTS] = {};
	int m_holdingBlockIndex = 0;
	AABB2 m_borderUVs;
};


