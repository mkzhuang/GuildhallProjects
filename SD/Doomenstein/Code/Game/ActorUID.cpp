#include "Game/ActorUID.hpp"

ActorUID::ActorUID()
{
}


ActorUID::ActorUID(int index, int salt)
{
	m_data = (index << 16) | (salt);
}


ActorUID const ActorUID::INVALID;


void ActorUID::Invalidate()
{
	*this = INVALID;
}


bool ActorUID::IsValid() const
{
	return *this != INVALID;
}


int ActorUID::GetIndex() const
{
	int data = m_data;
	return (data >> 16);
}


bool ActorUID::operator==(ActorUID const& other) const
{
	return m_data == other.m_data;
}


bool ActorUID::operator!=(ActorUID const& other) const
{
	return m_data != other.m_data;
}


