#include "Game/Player.hpp"
#include "Game/Map.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/EventSystem.hpp"

Player::Player(Map* map, Vec2 const& position, float speed)
	: m_map(map)
	, m_position(position)
	, m_speed(speed)
{
	Texture const* texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/DFSII/Assets.png");
	SpriteSheet* spriteSheet = new SpriteSheet(*texture, IntVec2(71, 19));
	m_spriteUV = spriteSheet->GetSpriteUVs(IntVec2(12, 0));

	SpriteAnimDefinition* moveSouth = new SpriteAnimDefinition(*spriteSheet, 12, 16, 0.5f,	SpriteAnimPlaybackType::LOOP);
	SpriteAnimDefinition* moveNorth = new SpriteAnimDefinition(*spriteSheet, 83, 87, 0.5f,	SpriteAnimPlaybackType::LOOP);
	SpriteAnimDefinition* moveEast	= new SpriteAnimDefinition(*spriteSheet, 154, 158, 0.5f,	SpriteAnimPlaybackType::LOOP);
	SpriteAnimDefinition* moveWest	= new SpriteAnimDefinition(*spriteSheet, 225, 229, 0.5f,	SpriteAnimPlaybackType::LOOP);
	m_animations[PlayerAnim::MOVE_SOUTH] = moveSouth;
	m_animations[PlayerAnim::MOVE_NORTH] = moveNorth;
	m_animations[PlayerAnim::MOVE_EAST]  = moveEast;
	m_animations[PlayerAnim::MOVE_WEST]  = moveWest;
	m_currentAnimation = PlayerAnim::MOVE_SOUTH;
}


void Player::Update(float deltaSeconds)
{
	m_movementInput = Vec2::ZERO;
	m_velocity = Vec2::ZERO;
	m_actionTimer += deltaSeconds;

	ExecutePlan(deltaSeconds);
	//HandleInput();
	UpdateTransform(deltaSeconds);
}


void Player::Render() const
{
	std::vector<Vertex_PCU> verts;
	AABB2 bodyBox(m_position - Vec2(0.5f, 0.5f), m_position + Vec2(0.5f, 0.5f));
	std::map<PlayerAnim, SpriteAnimDefinition*>::const_iterator iter = m_animations.find(m_currentAnimation);
	AABB2 const& spriteUVs = iter->second->GetSpriteDefAtTime(m_animationTimer).GetUVs();
	AddVertsForAABB2D(verts, bodyBox, Rgba8::WHITE, spriteUVs);
	g_theRenderer->DrawVertexArray(verts);
}


void Player::HandleInput()
{
	m_movementInput +=	g_theInput->IsKeyDown('W') ? Vec2::NORTH : Vec2::ZERO;
	m_movementInput +=	g_theInput->IsKeyDown('S') ? Vec2::SOUTH : Vec2::ZERO;
	m_movementInput +=	g_theInput->IsKeyDown('A') ? Vec2::WEST	 : Vec2::ZERO;
	m_movementInput +=	g_theInput->IsKeyDown('D') ? Vec2::EAST  : Vec2::ZERO;
	m_movementInput.ClampLength(1.f);
}


void Player::UpdateTransform(float deltaSeconds)
{
	if (m_movementInput.GetLengthSquared() != 0.f)
	{
		m_velocity = m_movementInput * m_speed;
		m_position += m_velocity * deltaSeconds;
	}

	Vec2 direction = m_velocity.GetNormalized();

	m_animationTimer = (direction == Vec2::ZERO) ? 0.f : (m_animationTimer + deltaSeconds);

	if (direction.x > 0.f && m_currentAnimation != PlayerAnim::MOVE_EAST)
	{
		m_animationTimer = 0.f;
		m_currentAnimation = PlayerAnim::MOVE_EAST;
	}
	else if (direction.x < 0.f && m_currentAnimation != PlayerAnim::MOVE_WEST)
	{
		m_animationTimer = 0.f;
		m_currentAnimation = PlayerAnim::MOVE_WEST;
	}
	else if (direction.y > 0.f && m_currentAnimation != PlayerAnim::MOVE_NORTH)
	{
		m_animationTimer = 0.f;
		m_currentAnimation = PlayerAnim::MOVE_NORTH;
	}
	else if (direction.y < 0.f && m_currentAnimation != PlayerAnim::MOVE_SOUTH)
	{
		m_animationTimer = 0.f;
		m_currentAnimation = PlayerAnim::MOVE_SOUTH;
	}
}


void Player::ExecutePlan(float deltaSeconds)
{
	if (m_currentActions.empty())
	{
		return;
	}

	Action const& action = m_currentActions.front();

	float dayFraction = fmodf(m_map->m_worldDay, 1.f);
	if (dayFraction < 0.25 || dayFraction > 0.9)
	{
		if (action.type == "moveTo")
		{
			if (!m_wayPoints.empty())
			{
				Vec2 nextWayPoint = m_wayPoints.front();
				Vec2 displacement = nextWayPoint - m_position;

				if (displacement.GetLengthSquared() == 0.f)
				{
					return;
				}
			}
			else
			{
				return;
			}
		}
	}

	if (action.type == "moveTo")
	{
		m_actionTimer = 0.f;
		if (m_wayPoints.empty())
		{
			Vec2 targetLocation = action.targetLocation;
			m_wayPoints = m_map->GetPathBetweenPositions(m_position, targetLocation);
			EventArgs args;
			std::string log = Stringf("Player is moving to %s(%.f,%.f)", action.targetName.c_str(), targetLocation.x, targetLocation.y);
			args.SetValue("log", log);
			CallHandler("addLog", args);
		}
		else
		{
			Vec2 nextWayPoint = m_wayPoints.front();
			Vec2 displacement = nextWayPoint - m_position;

			if (displacement.GetLengthSquared() == 0.f)
			{
				m_wayPoints.pop_front();
			}

			if (displacement.x != 0.f)
			{
				m_movementInput = displacement.x < 0.f ? Vec2::WEST : Vec2::EAST;
				if (fabsf(m_movementInput.x * m_speed * deltaSeconds) > fabsf(displacement.x))
				{
					m_movementInput.x = displacement.x / (m_speed * deltaSeconds);
				}
			}
			else if (displacement.y != 0.f)
			{
				m_movementInput = displacement.y < 0.f ? Vec2::SOUTH : Vec2::NORTH;
				if (fabsf(m_movementInput.y * m_speed * deltaSeconds) > fabsf(displacement.y))
				{
					m_movementInput.y = displacement.y / (m_speed * deltaSeconds);
				}
			}

			if (m_wayPoints.empty())
			{
				EventArgs args;
				std::string log = Stringf("Player arrived at %s(%.f,%.f)", action.targetName.c_str(), nextWayPoint.x, nextWayPoint.y);
				args.SetValue("log", log);
				CallHandler("addLog", args);
				m_currentActions.pop_front();
				return;
			}
		}
		return;
	}

	if (m_actionTimer < 1.f) return;
	if (action.type == "tradeItem")
	{
		std::string giveItem = action.preCondition.item;
		std::string giveString = giveItem;
		if (giveItem == "gold")
		{
			m_golds -= action.preCondition.value;
			if (action.preCondition.value == 1)
			{
				giveString = Stringf("%d gold", action.preCondition.value);
			}
			else
			{
				giveString = Stringf("%d golds", action.preCondition.value);
			}
		}
		else
		{
			std::vector<std::string>::iterator iter;
			for (iter = m_items.begin(); iter != m_items.end(); )
			{
				if (iter->data() == giveItem)
				{
					m_items.erase(iter);
					break;
				}
			}
		}

		std::string takeItem = action.effect.item;
		std::string takeString = takeItem;
		if (takeItem == "gold")
		{
			m_golds += action.effect.value;
			if (action.effect.value == 1)
			{
				takeString = Stringf("%d gold", action.effect.value);
			}
			else
			{
				takeString = Stringf("%d golds", action.effect.value);
			}
		}
		else
		{
			m_items.push_back(takeItem);
		}

		EventArgs args;

		std::string log = Stringf("Player traded %s with %s", takeString.c_str(), giveString.c_str());
		args.SetValue("log", log);
		CallHandler("addLog", args);
		m_currentActions.pop_front();
	}
	else if (action.type == "killSlime" || action.type == "killGoblin" || action.type == "killCrab")
	{
		std::string characterName;
		if (action.type == "killSlime")
		{
			characterName = "slime";
		}
		else if (action.type == "killGoblin")
		{
			characterName = "goblin";
		}
		else if (action.type == "killCrab")
		{
			characterName = "crab";
		}
		Character const* character = m_map->FindCharacter(characterName, action.targetIndex);

		m_currentHealth -= character->m_characterDef->m_damage;
		EventArgs damageArgs;
		std::string damageLog;
		if (character->m_characterDef->m_damage == 1)
		{
			damageLog = Stringf("Player took %.f damage", character->m_characterDef->m_damage);
		}
		else
		{
			damageLog = Stringf("Player took %.f damages", character->m_characterDef->m_damage);
		}
		damageArgs.SetValue("log", damageLog);
		CallHandler("addLog", damageArgs);

		std::string itemString = action.effect.item;
		if (action.effect.item == "gold")
		{
			m_golds += action.effect.value;
			if (action.effect.value == 1)
			{
				itemString = Stringf("%d gold", action.effect.value);
			}
			else 
			{
				itemString = Stringf("%d golds", action.effect.value);
			}
		}
		else
		{
			m_items.push_back(action.effect.item);
		}

		m_map->RemoveCharacter(characterName, action.targetIndex);

		EventArgs args;
		std::string log = Stringf("Player killed %s and got %s", characterName.c_str(), itemString.c_str());
		args.SetValue("log", log);
		CallHandler("addLog", args);
		m_currentActions.pop_front();
	}
	else if (action.type == "useItem")
	{
		for (int index = 0; index < (int)m_items.size(); index++)
		{
			std::string const& item = m_items[index];
			if (item == action.preCondition.item)
			{
				m_items.erase(m_items.begin() + index);
				break;
			}
		}

		if (action.preCondition.item == "potion")
		{
			m_currentHealth += 10;
			if (m_currentHealth > m_maxHealth) m_currentHealth = m_maxHealth;
			EventArgs args;
			args.SetValue("log", "Player used potion and healed 10 health");
			CallHandler("addLog", args);
		}
		m_currentActions.pop_front();
	}

	m_actionTimer = 0.f;
}


void Player::CreateRandomGoal()
{
	int roll = RNG.RollRandomIntInRange(0, 3);

	GameState goal;
	goal.key = "hasItem";
	if (roll == 0)
	{
		goal.item = "sword";
	}
	else if (roll == 1)
	{
		goal.item = "axe";
	}
	else if (roll == 2)
	{
		goal.item = "shield";
	}
	else if (roll == 3)
	{
		goal.item = "potion";
	}

	if (m_currentHealth / m_maxHealth < 0.9f)
	{
		goal.key = "useItem";
		goal.item = "potion";
	}

	m_currentGoal = goal;
}


