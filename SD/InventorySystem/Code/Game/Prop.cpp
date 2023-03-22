#include "Game/Prop.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"

Prop::Prop(Game* owner, Vec3 const& startPos, PropType type)
	: Entity(owner, startPos)
	, m_type(type)
{
	switch (m_type)
	{
	case PropType::CUBE:
		CreateCube();
		break;
	case  PropType::SPHERE:
		CreateSphere();
		break;
	case  PropType::GRID:
		CreateGrid();
		break;
	default:
		break;
	}
}


Prop::~Prop()
{
}


void Prop::Update(float deltaSeconds)
{
	switch (m_type)
	{
	case PropType::CUBE:
		m_angularVelocity.m_yawDegrees = 75.f;
		{float totalTime = m_game->GetTotalGameTime();
		float offset = SinDegrees(totalTime * 45.f) * 122.f;
		m_color = Rgba8(static_cast<unsigned char>(122 + offset), static_cast<unsigned char>(122 + offset), 255, 255);}
		break;
	case  PropType::SPHERE:
		m_angularVelocity.m_yawDegrees = 45.f;
		m_angularVelocity.m_pitchDegrees = 60.f;
		break;
	case  PropType::GRID:
		break;
	default:
		break;
	}
	m_orientationDegree += (m_angularVelocity * deltaSeconds);
}


void Prop::Render() const
{
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->SetModelMatrix(GetModelMatrix());
	g_theRenderer->SetModelColor(m_color);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->DrawVertexArray((int)m_verts.size(), m_verts.data());
}


void Prop::CreateCube()
{
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
	AABB3 cube = AABB3::ZERO_TO_ONE;
	cube.SetCenter(Vec3(0.f, 0.f, 0.f));

	Vec3 corners[8];
	cube.GetCorners(corners);
	Vec3 nearBL = corners[0];
	Vec3 nearBR = corners[1];
	Vec3 nearTR = corners[2];
	Vec3 nearTL = corners[3];
	Vec3 farBL = corners[4];
	Vec3 farBR = corners[5];
	Vec3 farTR = corners[6];
	Vec3 farTL = corners[7];

	// x forward, y left, z up
	AddVertsForQuad3D(m_verts, farBR, farBL, farTL, farTR, Rgba8::RED); //back quad +x
	AddVertsForQuad3D(m_verts, nearBL, nearBR, nearTR, nearTL, Rgba8::CYAN); //front quad -x
	AddVertsForQuad3D(m_verts, farBL, nearBL, nearTL, farTL, Rgba8::GREEN); //left quad +y
	AddVertsForQuad3D(m_verts, nearBR, farBR, farTR, nearTR, Rgba8::MAGENTA); //right quad -y
	AddVertsForQuad3D(m_verts, nearTL, nearTR, farTR, farTL, Rgba8::BLUE); //top quad +z
	AddVertsForQuad3D(m_verts, farBL, farBR, nearBR, nearBL, Rgba8::YELLOW); //bottom quad -z
}


void Prop::CreateSphere()
{
	m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
	m_color = Rgba8::WHITE;
	static const float longitudeSlices = 16.f;
	static const float latitudeSlices = 8.f;
	AddVertsForSphere3D(m_verts, Vec3::ZERO, 1.f, longitudeSlices, latitudeSlices, Rgba8::WHITE);
}


void Prop::CreateGrid()
{
	for (float distance = 1.f; distance <= 50.f; distance+= 1.f)
	{
		float halfWidth = 0.005f;
		Rgba8 xColor = Rgba8(200, 200, 200, 255);
		Rgba8 yColor = Rgba8(200, 200, 200, 255);
		if (fmodf(distance, 5.f) == 0.f) 
		{
			halfWidth = 0.015f;
			xColor = Rgba8::RED;
			yColor = Rgba8::GREEN;
		}
		// x lines
		AddVertsForAABB3D(m_verts, AABB3(Vec3(-50.f, -distance -halfWidth, -halfWidth), Vec3(50.f, -distance + halfWidth, halfWidth)), xColor);
		AddVertsForAABB3D(m_verts, AABB3(Vec3(-50.f, distance - halfWidth, -halfWidth), Vec3(50.f, distance + halfWidth, halfWidth)), xColor);
		// y lines
		AddVertsForAABB3D(m_verts, AABB3(Vec3(-distance - halfWidth, -50.f, -halfWidth), Vec3(-distance + halfWidth, 50.f, halfWidth)), yColor);
		AddVertsForAABB3D(m_verts, AABB3(Vec3(distance - halfWidth, -50.f, -halfWidth), Vec3(distance + halfWidth, 50.f, halfWidth)), yColor);
	}
	//origin
	AddVertsForAABB3D(m_verts, AABB3(Vec3(-0.025f, -50.f, -0.025f), Vec3(0.025f, 50.f, 0.025f)));
	AddVertsForAABB3D(m_verts, AABB3(Vec3(-50.f, -0.025f, -0.025f), Vec3(50.f, 0.025f, 0.025f)));
}


