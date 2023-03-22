#pragma once
#include "Engine/Core/EventHandlerBase.hpp"

template <class T>
 class EventHandler : public EventHandlerBase
{
public:
	using ObjectMemberFunc = bool (T::*)(EventArgs& args);

	template <class T>
	EventHandler(T* obj, ObjectMemberFunc functionPtr)
		: m_obj(obj)
		, m_function(functionPtr)
	{
	}

	~EventHandler() {}

	bool CallFunction(EventArgs& args) override
	{
		return (m_obj->*m_function)(args);
	}

	void* GetHandler() const override
	{
		return m_obj;
	}

	ObjectMemberFunc GetFunction()
	{
		return m_function;
	}

public:
	T* m_obj = nullptr;
	ObjectMemberFunc m_function;
};


