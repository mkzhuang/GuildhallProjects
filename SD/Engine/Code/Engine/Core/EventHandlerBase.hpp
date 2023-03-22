#pragma once
#include "Engine/Core/EngineCommon.hpp"

class EventHandlerBase
{
public:
	EventHandlerBase() {}
	virtual ~EventHandlerBase() {}
	
	virtual bool CallFunction(EventArgs& args) = 0;
	virtual void* GetHandler() const = 0;
};