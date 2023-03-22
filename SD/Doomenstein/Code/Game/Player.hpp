#pragma once
#include "Game/Controller.hpp"
#include "Engine/Renderer/Camera.hpp"

class Game;

class Player : public Controller
{
public:
	Player(Game* game, Vec3 const& pos, Camera* camera, Camera* UICamera);
	~Player();

	void Update(float deltaSeconds);
	void ActorUpdate(float deltaSeconds);
	void FreeFlyUpdate(float deltaSeconds);
	void KeyboardUpdate(float deltaSeconds);
	void ControllerUpdate(float deltaSeconds);
	void Render() const;

	Mat44 GetModelMatrix() const;

public:
	Game* m_game = nullptr;
	Camera* m_camera = nullptr;
	Camera* m_UICamera = nullptr;
	float m_animationTimer = 0.f;
	bool m_isAttacking = false;
	bool m_freeFlyCameraMode = false;
	Vec3 m_position;
	Vec3 m_velocity;
	Vec3 m_acceleration;
	EulerAngles m_orientationDegree;
	EulerAngles m_angularVelocity;
	float m_speed = 5.f;
	float m_mouseScale = 5.f;
	float m_sprintMultiplier = 3.f;
	float m_physicsRadius = 0.f;
	float m_cosmeticRadius = 0.f;

	int m_playerIndex = -1;
	int m_controllerIndex = -1;
	int m_kills = 0;
};


