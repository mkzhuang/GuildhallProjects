#include "Game/Controller.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"

Controller::Controller()
{

}


Controller::~Controller()
{

}


void Controller::Update(float deltaSeconds)
{
	UNUSED (deltaSeconds)
}


void Controller::Possess(Actor* actor)
{
	actor->OnPossessed(this);
}



void Controller::UnPossess(Actor* actor)
{
	actor->OnUnpossessed(this);
}


Actor* Controller::GetActor() const
{
	return m_map->FindActorByUID(m_actorUID);
}


