#include "Game/Planner.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Player.hpp"
#include "Game/Character.hpp"
#include "Engine/Core/EventSystem.hpp"

void Planner::InitializePossibleActions(std::string const& xmlFilePath)
{
	XmlDocument doc;
	doc.LoadFile(xmlFilePath.c_str());

}


void Planner::MakePlan(Map* map, std::vector<GameState> const& currentState, GameState const& targetState, std::deque<Action>& actions)
{
	std::deque<GameState> stateQueue;
	stateQueue.push_back(targetState);
	while (!stateQueue.empty())
	{
		GameState currentTargetState = stateQueue.front();

		bool currentStateIsTarget = false;
		int currentGold = 0;
		for (GameState const& state : currentState)
		{
			if (state.key == currentTargetState.key && state.value >= currentTargetState.value && state.item == currentTargetState.item)
			{
				currentStateIsTarget = true;
			}
			if (state.key == "hasItem" && state.item == "gold")
			{
				currentGold = state.value;
			}
		}

		if (currentStateIsTarget) 
		{
			stateQueue.pop_front();
			continue;
		}

		if (currentTargetState.key == "hasItem")
		{
			if (currentTargetState.item == "gold")
			{
				int neededGold = currentTargetState.value - currentGold <= 0 ? 0 : currentTargetState.value - currentGold;
				std::vector<Action> killActions;
				std::vector<Action> moveActions;
				while (neededGold > 0)
				{
					int random = RNG.RollRandomIntInRange(0, 2);
					if (random == 0)
					{
						Character const* slime = map->AddCharacter("slime");
						Action movement;
						movement.type = "moveTo";
						movement.targetName = "slime";
						movement.targetLocation = slime->m_position;
						moveActions.push_back(movement);

						Action action;
						action.type = "killSlime";
						action.targetIndex = slime->m_characterIndex;
						action.effect.key = "hasItem";
						action.effect.value = 1;
						action.effect.item = "gold";
						killActions.push_back(action);

						neededGold -= 1;
					}
					else if (random == 1)
					{
						Character const* goblin = map->AddCharacter("goblin");
						Action movement;
						movement.type = "moveTo";
						movement.targetName = "goblin";
						movement.targetLocation = goblin->m_position;
						moveActions.push_back(movement);

						Action action;
						action.type = "killGoblin";
						action.targetIndex = goblin->m_characterIndex;
						action.effect.key = "hasItem";
						action.effect.value = 2;
						action.effect.item = "gold";
						killActions.push_back(action);

						neededGold -= 2;
					}
					else if (random == 2)
					{
						Character const* crab = map->AddCharacter("crab");
						Action movement;
						movement.type = "moveTo";
						movement.targetName = "crab";
						movement.targetLocation = crab->m_position;
						moveActions.push_back(movement);

						Action action;
						action.type = "killCrab";
						action.targetIndex = crab->m_characterIndex;
						action.effect.key = "hasItem";
						action.effect.value = 5;
						action.effect.item = "gold";
						killActions.push_back(action);

						neededGold -= 5;
					}
				}

				std::deque<Action> killActionQueue;
				Vec2 playerPosition = map->m_player->m_position;
				while (!moveActions.empty())
				{
					float bestDistance = 999999.f;
					int bestIndex = 0;
					for (int index = 0; index < (int)moveActions.size(); index++)
					{
						Action const& action = moveActions[index];
						float distance = (action.targetLocation - playerPosition).GetLengthSquared();
						if (distance < bestDistance)
						{
							bestDistance = distance;
							bestIndex = index;
						}
					}
					playerPosition = moveActions[bestIndex].targetLocation;
					killActionQueue.push_front(moveActions[bestIndex]);
					killActionQueue.push_front(killActions[bestIndex]);
					moveActions.erase(moveActions.begin() + bestIndex);
					killActions.erase(killActions.begin() + bestIndex);
				}

				while (!killActionQueue.empty())
				{
					actions.push_front(killActionQueue.front());
					killActionQueue.pop_front();
				}
			}
			else
			{
				Character const* character = map->FindCharacterWithItem(currentTargetState.item);
				std::string const& item = character->m_characterDef->m_item;
				std::string const& wantedItem = character->m_characterDef->m_wantItem;
				GameState state;
				if (ContainsSubstring(wantedItem, "gold"))
				{
					int goldNeeded = atoi(SplitStringOnDelimiter(wantedItem, ' ')[0].c_str());
					state.value = goldNeeded;
					state.item = "gold";
				}
				else
				{
					state.item = wantedItem;

				}
				state.key = "hasItem";
				stateQueue.push_back(state);

				Action action;
				action.type = "tradeItem";
				action.preCondition = state;
				action.effect.key = "hasItem";
				action.effect.item = item;
				actions.push_front(action);

				Action movement;
				movement.type = "moveTo";
				movement.targetName = character->m_characterDef->m_name;
				movement.targetLocation = character->m_position;
				actions.push_front(movement);
			}
			stateQueue.pop_front();
		}
		else if (currentTargetState.key == "useItem")
		{
			GameState state;
			state.key = "hasItem";
			state.item = currentTargetState.item;
			stateQueue.push_back(state);

			Action action;
			action.type = "useItem";
			action.preCondition = state;
			actions.push_front(action);

			stateQueue.pop_front();
		}
	}
}


